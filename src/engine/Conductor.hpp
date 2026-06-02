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
    uint32_t max_retries;
    RpcJsonClient rpc_client;

    std::jthread polling_thread;
    std::mutex result_mutex;
    std::mutex wake_mutex;
    std::condition_variable_any wake_cv;
    std::atomic_bool needs_refresh{false};
    std::atomic_uint32_t consecutive_failures{0};
    std::atomic_bool critical_failure{false};
    std::optional<block_assembly::Result> last_result;

    static constexpr auto POLL_INTERVAL = std::chrono::seconds(5);

    auto poll_loop(const std::stop_token& token) -> void;
    auto fetch_and_update_template() -> void;
    auto on_solution(const BlockHeader& header) -> void;

  public:
    explicit Conductor(const core::args::CliArgs& args);
    auto start() -> bool;
    auto stop() -> void;
    auto is_failed() const -> bool;
};

template<Hasher H>
Conductor<H>::Conductor(const core::args::CliArgs& args):
    engine{args.threads}, max_retries{args.retry},
    rpc_client{RpcConfig{
      .port = args.rpc_port,
      .timeout_sec = args.timeout,
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
        auto failures = consecutive_failures.fetch_add(1) + 1;
        Logger::instance().error(
            "Error fetching block template ({}/{}): {}",
            failures,
            max_retries,
            result.error().message
        );
        if (failures >= max_retries) {
            Logger::instance().error(
                "Max consecutive failures reached, shutting down"
            );
            critical_failure.store(true);
        }
        return;
    }

    consecutive_failures.store(0);

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
    while (!token.stop_requested() && !critical_failure.load()) {
        fetch_and_update_template();
        if (critical_failure.load()) {
            break;
        }
        needs_refresh.store(false);
        {
            std::unique_lock lock{wake_mutex};
            wake_cv.wait_for(lock, POLL_INTERVAL, [&] -> bool {
                return token.stop_requested() || needs_refresh.load()
                    || critical_failure.load();
            });
        }
        Logger::instance().info("Hashrate: {} MH/s", engine.get_hashrate());
    }
}

template<Hasher H> auto Conductor<H>::start() -> bool {
    auto test = rpc_client.ping();
    if (!test) {
        Logger::instance().error(
            "Failed to connect to RPC server at {}:{}: {}",
            rpc_client.get_host(),
            rpc_client.get_port(),
            test.error().message
        );
        return false;
    }
    this->fetch_and_update_template();
    polling_thread = std::jthread{[this](const std::stop_token& token) -> void {
        poll_loop(token);
    }};
    engine.start();
    return true;
}

template<Hasher H> auto Conductor<H>::stop() -> void {
    needs_refresh.store(true);
    wake_cv.notify_all();
    polling_thread = std::jthread{};
    engine.stop();
}

template<Hasher H> auto Conductor<H>::is_failed() const -> bool {
    return critical_failure.load();
}

template<Hasher H>
auto Conductor<H>::on_solution(const BlockHeader& header) -> void {
    Logger::instance().info("Solution found! Nonce: {}", header.nonce);
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
        auto failures = consecutive_failures.fetch_add(1) + 1;
        Logger::instance().error(
            "Error submitting block ({}/{}): {}",
            failures,
            max_retries,
            submit_result.error().message
        );
        if (failures >= max_retries) {
            Logger::instance().error(
                "Max consecutive failures reached, shutting down"
            );
            critical_failure.store(true);
        }
    } else {
        consecutive_failures.store(0);
        Logger::instance().info(
            "Block {} submitted successfully!", last_result->height
        );
        Logger::instance().info(
            "Reward: {} satoshis mined to {}",
            last_result->reward,
            rpc_client.get_address()
        );
        Logger::instance().info(
            "----------------------------------------------------------------"
        );
    }
    needs_refresh.store(true);
    wake_cv.notify_all();
}

Conductor(const core::args::CliArgs&) -> Conductor<DefaultHasher>;
