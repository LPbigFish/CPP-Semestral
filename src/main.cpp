#include "core/Args.hpp"
#include "engine/Conductor.hpp"

#include <atomic>
#include <csignal>
#include <cstdint>
#include <expected>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

auto main(int argc, char** argv) -> int32_t {
    static std::atomic_bool running{true};

    std::expected<core::args::CliArgs, std::string> result;
    {
        std::vector<std::string_view> args_vector(argv, argv + argc);

        result = core::args::parse_args(argc, args_vector);
    }

    if (!result) {
        if (result.error().starts_with("Usage:")) {
            std::println("{}", result.error());
            return 0;
        }
        std::println(stderr, "error: {}", result.error());
        return 1;
    }

    const auto& args = *result;

    if (args.verbose) {
        std::println("address: {}", args.address);
        std::println(
            "rpc: {}@{}:{}", args.rpc_username, args.rpc_host, args.rpc_port
        );
        std::println("threads: {}", args.threads);
    }

    std::signal(SIGINT, [](int) { running.store(false); });
    std::signal(SIGTERM, [](int) { running.store(false); });

    Conductor conductor{args};
    conductor.start();

    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::println("\nShutting down...");
    conductor.stop();

    return 0;
}
