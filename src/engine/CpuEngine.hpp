#pragma once

#include "../Logger.hpp"
#include "../core/BlockHeader.hpp"
#include "../hashers/Hasher.hpp"
#include "../mining/MiningJob.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <ranges>
#include <stop_token>
#include <thread>
#include <vector>

template<Hasher H> class CpuEngine {
    std::vector<std::jthread> threads;
    MiningJob current_job;
    std::function<void(const BlockHeader&)> on_solution;
    std::mutex job_lock;
    uint32_t num_threads{1};
    std::stop_source stop_source;
    std::atomic_bool solution_found{false};
    std::atomic<uint64_t> hashes_computed{0};
    bool running{false};

  public:
    CpuEngine() = default;
    CpuEngine(const CpuEngine& engine) = delete;
    CpuEngine(CpuEngine&& engine) noexcept = delete;
    auto operator=(const CpuEngine& other) -> CpuEngine& = delete;
    auto operator=(CpuEngine&& other) noexcept -> CpuEngine& = delete;

    explicit CpuEngine(uint32_t num_threads): num_threads{num_threads} {}

    ~CpuEngine() {
        stop();
    }

    auto start() -> void {
        stop();
        solution_found.store(false);
        hashes_computed.store(0);
        stop_source = std::stop_source();
        threads.clear();

        Logger::instance().debug("Starting {} mining threads", num_threads);

        for (uint32_t i = 0; i < num_threads; i++) {
            threads.emplace_back(
                &CpuEngine::work, this, stop_source.get_token(), i
            );
        }
        running = true;
    }

    auto stop() -> void {
        if (stop_source.stop_possible()) {
            stop_source.request_stop();
        }
        running = false;

        threads.clear();
        Logger::instance().debug("Mining engine stopped");
    }

    auto submit_job(const MiningJob& job) -> void {
        std::lock_guard lock{job_lock};
        current_job = job;
        Logger::instance().debug(
            "Job submitted: version={} bits={} time={}",
            job.block_header.version,
            job.block_header.bits,
            job.block_header.time
        );
    }

    auto solution_callback(std::function<void(const BlockHeader&)> callback)
        -> void {
        on_solution = std::move(callback);
    }

    auto is_running() const -> bool {
        return running;
    }

    auto get_hashrate() -> double {
        if (!running) {
            return 0.0;
        }
        uint64_t hashes = hashes_computed.exchange(0);
        return static_cast<double>(hashes) / 1'000'000.0; // MH/s
    }

  private:
    auto work(const std::stop_token& token, uint32_t thread_id) -> void {
        H local_hasher{};
        uint64_t local_count{0};

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

        Logger::instance().debug(
            "Thread {} starting, nonce range [{}..]", thread_id, nonce
        );

        while (!token.stop_requested() && !solution_found.load()) {
            auto nonce_le = endian::to_le_bytes(nonce);
            std::ranges::copy(nonce_le, tail.end() - sizeof(uint32_t));

            local_hasher.restore_state();
            local_hasher.update(tail);
            auto first_iter{local_hasher.finalize()};

            auto first_iter_bytes{first_iter.to_internal_bytes()};

            auto final_iter{H::hash_bytes(first_iter_bytes)};

            local_count++;

            if (final_iter < job.target) {
                if (!solution_found.exchange(true)) {
                    BlockHeader solution{job.block_header};
                    solution.nonce = nonce;
                    Logger::instance().debug(
                        "Thread {} found solution at nonce {}", thread_id, nonce
                    );
                    hashes_computed.fetch_add(
                        local_count & 0xFFFF, std::memory_order_relaxed
                    );
                    on_solution(solution);
                }
                return;
            }

            if (nonce > UINT32_MAX - num_threads) {
                Logger::instance().debug(
                    "Thread {} exhausted nonce space", thread_id
                );
                break;
            }

            if ((local_count & 0xFFFF) == 0) {
                hashes_computed.fetch_add(0xFFFF, std::memory_order_relaxed);
            }

            nonce += num_threads;
        }
    }
};
