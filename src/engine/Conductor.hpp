#pragma once
#include "../Logger.hpp"
#include "../core/Args.hpp"
#include "../hashers/OpensslHasher.hpp"
#include "../mining/BlockAssembly.hpp"
#include "../networking/RpcJsonClient.hpp"
#include "CpuEngine.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>

using DefaultHasher = OpensslHasher;

template<Hasher H> class Conductor {
    CpuEngine<H> engine;
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

template<Hasher H>
Conductor<H>::Conductor(const core::args::CliArgs& args):
    engine{args.threads}, rpc_client{RpcConfig{
                            .port = args.rpc_port,
                            .host = args.rpc_host,
                            .username = args.rpc_username,
                            .password = args.rpc_password,
                            .address = args.address,
                            .network = args.network,
                          }} {
    engine.solution_callback([this](const BlockHeader& header) -> void {
        on_solution(header);
    });
}

template<Hasher H> auto Conductor<H>::fetch_and_update_template() -> void {
    auto result = rpc_client.get_block_template();
    if (!result) {
        Logger::instance().error(
            "Error fetching block template: {}", result.error().message
        );
        return;
    }

    auto& tmpl = *result;

    auto assembly_result = block_assembly::assemble(tmpl);

    {
        std::lock_guard lock{result_mutex};

        if (!last_result
            || assembly_result.job.block_header.previous_hash
                   != last_result->job.block_header.previous_hash) {
            Logger::instance().info("New block detected, updating template...");
        } else {
            return;
        }
        last_result = assembly_result;

        if (engine.is_running()) {
            engine.stop();
            engine.submit_job(last_result->job);
            engine.start();
        } else {
            engine.submit_job(last_result->job);
        }
    }
}

template<Hasher H>
auto Conductor<H>::poll_loop(const std::stop_token& token) -> void {
    while (!token.stop_requested()) {
        fetch_and_update_template();
        needs_refresh.store(false);
        {
            std::unique_lock lock{wake_mutex};
            wake_cv.wait_for(lock, POLL_INTERVAL, [&] -> bool {
                return token.stop_requested() || needs_refresh.load();
            });
        }
        Logger::instance().info("Hashrate: {} MH/s", engine.get_hashrate());
    }
}

template<Hasher H> auto Conductor<H>::start() -> void {
    this->fetch_and_update_template();
    polling_thread = std::jthread{[this](const std::stop_token& token) -> void {
        poll_loop(token);
    }};
    engine.start();
}

template<Hasher H> auto Conductor<H>::stop() -> void {
    needs_refresh.store(true);
    wake_cv.notify_all();
    polling_thread = std::jthread{};
    engine.stop();
}

template<Hasher H>
auto Conductor<H>::on_solution(const BlockHeader& header) -> void {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    Logger::instance().info(
        "Solution found! Nonce: {} | {}", header.nonce, std::ctime(&time_t)
    );
    Logger::instance().info("Block hash: {}", header.hash().to_hex());
    std::lock_guard lock{result_mutex};
    if (!last_result) {
        Logger::instance().error(
            "No block template available, cannot submit block"
        );
        return;
    }
    auto block_data = block_assembly::build_submission(header, *last_result);
    auto submit_result = rpc_client.submit_block(block_data);
    if (!submit_result) {
        Logger::instance().error(
            "Error submitting block: {}", submit_result.error().message
        );
    } else {
        Logger::instance().info(
            "Block {} submitted successfully!", last_result->height
        );
        Logger::instance().info(
            "Reward: {} satoshis mined to {}",
            last_result->reward,
            rpc_client.get_address()
        );
        Logger::instance().info("------------------------------");
    }
    needs_refresh.store(true);
    wake_cv.notify_all();
}

Conductor(const core::args::CliArgs&) -> Conductor<DefaultHasher>;
