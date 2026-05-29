#include "Args.hpp"

#include <format>
#include <span>
#include <string_view>

namespace core::args {
namespace {

auto parse_uint(std::string_view value, std::string_view flag)
    -> std::expected<uint32_t, std::string> {
    try {
        auto v = std::stoul(std::string{value});
        return static_cast<uint32_t>(v);
    } catch (const std::exception&) {
        return std::unexpected(
            std::format("Invalid value for {}: {}", flag, value)
        );
    }
}

auto apply_arg(CliArgs& args, std::string_view flag, std::string_view value)
    -> std::expected<void, std::string> {
    if (flag == "--rpc-host") {
        args.rpc_host = value;
        return {};
    }
    if (flag == "--rpc-username") {
        args.rpc_username = value;
        return {};
    }
    if (flag == "--rpc-password") {
        args.rpc_password = value;
        return {};
    }
    if (flag == "--address") {
        args.address = value;
        return {};
    }

    auto num = parse_uint(value, flag);
    if (!num) {
        return std::unexpected(num.error());
    }

    if (flag == "--rpc-port") {
        args.rpc_port = static_cast<uint16_t>(*num);
    } else if (flag == "--retry") {
        args.retry = *num;
    } else if (flag == "--timeout") {
        args.timeout = *num;
    } else if (flag == "--threads") {
        args.threads = *num;
    }

    return {};
}

auto is_value_arg(std::string_view arg) -> bool {
    return arg == "--rpc-host" || arg == "--rpc-port" || arg == "--rpc-username"
        || arg == "--rpc-password" || arg == "--retry" || arg == "--timeout"
        || arg == "--threads" || arg == "--address";
}

} // namespace

auto help_text() -> std::string {
    return "Usage: ./miner --address <address> [options]\n"
           "Options:\n"
           "  --rpc-host <host>         RPC server host (default: 127.0.0.1)\n"
           "  --rpc-port <port>         RPC server port (default: 18443)\n"
           "  --rpc-username <username> RPC server username (default: admin)\n"
           "  --rpc-password <password> RPC server password (default: "
           "password)\n"
           "  --retry <seconds>         Seconds to wait before retrying "
           "after an error (default: 5)\n"
           "  --timeout <seconds>       Seconds to wait for a solution "
           "before giving up (default: 30)\n"
           "  --threads <number>        Number of mining threads to use "
           "(default: number of CPU cores)\n"
           "  --verbose                 Enable verbose logging";
}

auto parse_args(int argc, std::span<std::string_view> argv)
    -> std::expected<CliArgs, std::string> {
    CliArgs args;

    for (int i = 1; i < argc; ++i) {
        auto arg = std::string_view{argv[i]};

        if (arg == "--help" || arg == "-h") {
            return std::unexpected(help_text());
        }

        if (arg == "--verbose") {
            args.verbose = true;
            continue;
        }

        if (!is_value_arg(arg)) {
            return std::unexpected(std::format("Unknown argument: {}", arg));
        }

        if (++i >= argc) {
            return std::unexpected(std::format("Missing value for {}", arg));
        }

        auto result = apply_arg(args, arg, argv[i]);
        if (!result) {
            return std::unexpected(result.error());
        }
    }

    if (args.address.empty()) {
        return std::unexpected("--address is required");
    }

    return args;
}

} // namespace core::args
