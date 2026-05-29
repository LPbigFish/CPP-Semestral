#include "CpuEngine.hpp"
#include "../core/OpensslHasher.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <ranges>
#include <span>
#include <stop_token>
#include <utility>

CpuEngine::CpuEngine(uint32_t num_threads): num_threads{num_threads} {}

auto CpuEngine::start() -> void {
    stop();
    solution_found.store(false);
    stop_source = std::stop_source();
    threads.clear();

    for (uint32_t i = 0; i < num_threads; i++) {
        threads.emplace_back(
            &CpuEngine::work, this, stop_source.get_token(), i
        );
    }
    running = true;
}

auto CpuEngine::stop() -> void {
    if (stop_source.stop_possible()) {
        stop_source.request_stop();
    }
    running = false;

    threads.clear();
}

auto CpuEngine::submit_job(const MiningJob& job) -> void {
    std::lock_guard lock{job_lock};
    current_job = job;
}

auto CpuEngine::solution_callback(
    std::function<void(const BlockHeader&)> callback
) -> void {
    on_solution = std::move(callback);
}

auto CpuEngine::work(const std::stop_token& token, uint32_t thread_id) -> void {
    OpensslHasher local_hasher{};

    MiningJob job;
    {
        std::lock_guard lock{job_lock};
        job = current_job;
    }

    auto header_bytes = job.block_header.serialize();
    std::span<const uint8_t> head{header_bytes.data(), 64};
    std::array<uint8_t, 16> tail{};

    std::ranges::copy(header_bytes | std::views::drop(64), tail.begin());

    local_hasher.update(head);
    local_hasher.save_state();

    uint32_t nonce{job.block_header.nonce + thread_id};

    while (!token.stop_requested() && !solution_found.load()) {
        auto nonce_le = endian::to_le_bytes(nonce);
        std::ranges::copy(nonce_le, tail.end() - sizeof(uint32_t));

        local_hasher.restore_state();
        local_hasher.update(tail);
        auto first_iter{local_hasher.finalize()};

        auto first_iter_bytes{first_iter.to_internal_bytes()};

        auto final_iter{OpensslHasher::hash_bytes(first_iter_bytes)};

        if (final_iter < job.target) {
            if (!solution_found.exchange(true)) {
                BlockHeader solution{job.block_header};
                solution.nonce = nonce;
                on_solution(solution);
            }
            return;
        }

        if (nonce > UINT32_MAX - num_threads) {
            break;
        }

        nonce += num_threads;
        if (nonce < thread_id) {
            break;
        }
    }
}
