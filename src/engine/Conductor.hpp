#pragma once
#include "../core/Args.hpp"
#include "../mining/BlockAssembly.hpp"
#include "../networking/RpcJsonClient.hpp"
#include "CpuEngine.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>

class Conductor {
    CpuEngine engine;
    RpcJsonClient rpc_client;

    std::jthread polling_thread;
    std::mutex result_mutex;
    std::mutex wake_mutex;
    std::condition_variable_any wake_cv;
    std::atomic_bool needs_refresh{false};
    std::optional<block_assembly::Result> last_result;

    static constexpr auto POLL_INTERVAL = std::chrono::seconds(5);

    auto poll_loop(const std::stop_token& token) -> void;
    auto fetch_and_update_template() -> void;
    auto on_solution(const BlockHeader& header) -> void;

  public:
    explicit Conductor(const core::args::CliArgs& args);
    auto start() -> void;
    auto stop() -> void;
};
