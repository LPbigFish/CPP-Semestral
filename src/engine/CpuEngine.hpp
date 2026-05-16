#pragma once

#include "../core/Hasher.hpp"
#include "MiningEngine.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>

class CpuEngine: public MiningEngine {
    std::unique_ptr<Hasher> hasher_init;
    uint8_t num_threads{1};
    std::vector<std::jthread> threads;
    std::function<void(const BlockHeader&)> on_solution;
    std::mutex job_lock;
    MiningJob current_job;
    std::stop_source stop_source;
    std::atomic_bool solution_found{false};

    auto work(const std::stop_token& token, uint8_t thread_id) -> void;

  public:
    CpuEngine() = default;
    CpuEngine(const Hasher& hasher, uint8_t num_threads);
    CpuEngine(const CpuEngine& engine);
    CpuEngine(CpuEngine&& engine) noexcept;
    auto operator=(const CpuEngine& other) -> CpuEngine&;
    auto operator=(CpuEngine&& other) noexcept -> CpuEngine&;

    ~CpuEngine() override {
        stop();
    }

    auto start() -> void override;
    auto stop() -> void override;
    auto submit_job(const MiningJob& job) -> void override;
    auto solution_callback(std::function<void(const BlockHeader&)> callback)
        -> void override;
};
