#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <thread>

namespace core::args {

/*
struct RpcConfig {
    std::string host;
    uint16_t port;
    std::string username;
    std::string password;
    std::string address;
};
*/

struct CliArgs {
    std::string rpc_host{"127.0.0.1"};
    uint16_t rpc_port{18443};
    std::string rpc_username{"admin"};
    std::string rpc_password{"password"};
    uint32_t retry{5};
    uint32_t timeout{30};
    bool verbose{false};

    std::string address;
    uint32_t threads{std::thread::hardware_concurrency()};
};

auto help_text() -> std::string;

auto parse_args(int argc, std::span<std::string_view> argv)
    -> std::expected<CliArgs, std::string>;

} // namespace core::args
