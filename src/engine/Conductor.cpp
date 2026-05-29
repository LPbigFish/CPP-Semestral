#include "Conductor.hpp"
#include <mutex>
#include <print>
#include <stop_token>

Conductor::Conductor(const core::args::CliArgs& args):
    engine{args.threads}, rpc_client{RpcConfig{
                            .host = args.rpc_host,
                            .port = args.rpc_port,
                            .username = args.rpc_username,
                            .password = args.rpc_password,
                            .address = args.address,
                          }} {
    engine.solution_callback([this](const BlockHeader& header) -> void {
        on_solution(header);
    });
}

auto Conductor::fetch_and_update_template() -> void {
    auto result = rpc_client.get_block_template();
    if (!result) {
        std::println(
            stderr, "Error fetching block template: {}", result.error().message
        );
        return;
    }

    auto& tmpl = *result;

    auto assembly_result = block_assembly::assemble(tmpl);

    {
        std::lock_guard lock{result_mutex};

        if (!last_result
            || assembly_result.job.block_header.merkle_root
                   != last_result->job.block_header.merkle_root
            || assembly_result.job.block_header.previous_hash
                   != last_result->job.block_header.previous_hash) {
            std::println("New block detected, updating template...");
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

auto Conductor::poll_loop(const std::stop_token& token) -> void {
    while (!token.stop_requested()) {
        fetch_and_update_template();
        needs_refresh.store(false);
        {
            std::unique_lock lock{wake_mutex};
            wake_cv.wait_for(lock, POLL_INTERVAL, [&] {
                return token.stop_requested() || needs_refresh.load();
            });
        }
    }
}

auto Conductor::start() -> void {
    this->fetch_and_update_template();
    polling_thread = std::jthread{[this](const std::stop_token& token) -> void {
        poll_loop(token);
    }};
    engine.start();
}

auto Conductor::stop() -> void {
    needs_refresh.store(true);
    wake_cv.notify_all();
    polling_thread = std::jthread{};
    engine.stop();
}

auto Conductor::on_solution(const BlockHeader& header) -> void {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::println(
        "Solution found! Nonce: {} | {}", header.nonce, std::ctime(&time_t)
    );
    std::println("Block hash: {}", header.hash().to_hex());
    std::println("Target: {}", BlockHeader::get_target(header.bits).to_hex());
    std::lock_guard lock{result_mutex};
    if (!last_result) {
        std::println(
            stderr, "No block template available *SOMEHOW*, cannot submit block"
        );
        return;
    }
    auto block_data = block_assembly::build_submission(header, *last_result);
    auto submit_result = rpc_client.submit_block(block_data);
    if (!submit_result) {
        std::println(
            stderr, "Error submitting block: {}", submit_result.error().message
        );
    } else {
        std::println("Block {} submitted successfully!", last_result->height);
        std::println(
            "Reward: {} satoshis mined to {}",
            last_result->reward,
            rpc_client.get_address()
        );
        std::println("------------------------------");
    }
    needs_refresh.store(true);
    wake_cv.notify_all();
}
