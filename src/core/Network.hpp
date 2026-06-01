#pragma once

#include "../core/utils.hpp"
#include <cstdint>
#include <variant>

namespace core::network {

struct Mainnet {};

struct Testnet {};

struct Regtest {};

using Network = std::variant<Mainnet, Testnet, Regtest>;

constexpr auto p2pkh_prefix(const Network& network) -> uint8_t {
    return std::visit(
        core::overloaded{
          [](Mainnet) -> uint8_t { return 0x00; },
          [](Testnet) -> uint8_t { return 0x6F; },
          [](Regtest) -> uint8_t { return 0x6F; }
        },
        network
    );
}

constexpr auto p2sh_prefix(const Network& network) -> uint8_t {
    return std::visit(
        core::overloaded{
          [](Mainnet) -> uint8_t { return 0x05; },
          [](Testnet) -> uint8_t { return 0xC4; },
          [](Regtest) -> uint8_t { return 0xC4; }
        },
        network
    );
}
} // namespace core::network
