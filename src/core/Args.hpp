#pragma once

#include "Network.hpp"
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <thread>

namespace net = core::network;

namespace core::args {

enum class EngineType : uint8_t {
    Own,
    Openssl,
};

struct CliArgs {
    std::string rpc_host{"127.0.0.1"};
    uint16_t rpc_port{18443};
    std::string rpc_username{"admin"};
    std::string rpc_password{"password"};
    uint32_t retry{5};
    uint32_t timeout{30};
    bool verbose{false};
    bool benchmark{false};

    std::string address;
    net::Network network{net::Regtest{}};
    uint32_t threads{std::thread::hardware_concurrency()};
    EngineType engine{EngineType::Openssl};
};

auto help_text() -> std::string;

auto parse_args(int argc, std::span<std::string_view> argv)
    -> std::expected<CliArgs, std::string>;

} // namespace core::args
