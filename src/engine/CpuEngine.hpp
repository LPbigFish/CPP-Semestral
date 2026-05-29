#pragma once

#include "../core/BlockHeader.hpp"
#include "../mining/MiningJob.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>

class CpuEngine {
    std::vector<std::jthread> threads;
    MiningJob current_job;
    std::function<void(const BlockHeader&)> on_solution;
    std::mutex job_lock;
    uint32_t num_threads{1};
    std::stop_source stop_source;
    std::atomic_bool solution_found{false};
    bool running{false};

    auto work(const std::stop_token& token, uint32_t thread_id) -> void;

  public:
    CpuEngine() = default;
    explicit CpuEngine(uint32_t num_threads);
    CpuEngine(const CpuEngine& engine);
    CpuEngine(CpuEngine&& engine) noexcept;
    auto operator=(const CpuEngine& other) -> CpuEngine&;
    auto operator=(CpuEngine&& other) noexcept -> CpuEngine&;

    ~CpuEngine() {
        stop();
    }

    auto start() -> void;
    auto stop() -> void;
    auto submit_job(const MiningJob& job) -> void;
    auto solution_callback(std::function<void(const BlockHeader&)> callback)
        -> void;

    auto is_running() const -> bool {
        return running;
    }
};
