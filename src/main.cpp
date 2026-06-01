#include "Logger.hpp"
#include "core/Args.hpp"
#include "engine/Conductor.hpp"
#include "hashers/OwnHasher.hpp"

#include <atomic>
#include <csignal>
#include <cstdint>
#include <expected>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>

using AnyConductor
    = std::variant<Conductor<OwnHasher>, Conductor<OpensslHasher>>;

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

    Logger::instance().init(args.verbose);

    Logger::instance().debug("address: {}", args.address);
    Logger::instance().debug(
        "rpc: {}@{}:{}", args.rpc_username, args.rpc_host, args.rpc_port
    );
    Logger::instance().debug("threads: {}", args.threads);

    std::signal(SIGINT, [](int) -> void { running.store(false); });
    std::signal(SIGTERM, [](int) -> void { running.store(false); });

    auto make_conductor = [&]() -> AnyConductor {
        if (args.engine == core::args::EngineType::Own) {
            Logger::instance().info("Using own hasher");
            return AnyConductor{std::in_place_type<Conductor<OwnHasher>>, args};
        }
        Logger::instance().info("Using OpenSSL hasher");
        return AnyConductor{std::in_place_type<Conductor<OpensslHasher>>, args};
    };

    auto conductor = make_conductor();

    std::visit([](auto& c) -> auto { c.start(); }, conductor);

    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Logger::instance().info("Shutting down...");
    std::visit([](auto& c) -> auto { c.stop(); }, conductor);

    return 0;
}
