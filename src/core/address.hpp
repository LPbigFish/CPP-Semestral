#pragma once

#include "base58.hpp"
#include <cstdint>
#include <expected>
#include <string_view>
#include <vector>


namespace core::address {
enum class address_parse_error : std::uint8_t {
    InvalidLength,
    InvalidBase58Input
};

constexpr auto craft_p2pkh_scriptpubkey(std::string_view input_address)
    -> std::expected<std::vector<uint8_t>, address_parse_error> {
    auto decoded = base58::decode_base58(input_address);

    if (!decoded.has_value()) {
        return std::unexpected(address_parse_error::InvalidBase58Input);
    }

    std::vector<uint8_t> bytes(decoded.value().begin(), decoded.value().end());

    if (bytes.size() != 25) {
        return std::unexpected(
            address_parse_error::InvalidLength
        );
    }

    // skip chain id a checksum
    std::vector<uint8_t> pubkey_hash(bytes.begin() + 1, bytes.begin() + 21);

    std::vector<uint8_t> script;
    script.reserve(25);

    script.push_back(0x76);
    script.push_back(0xA9);
    script.push_back(0x14);
    script.insert(script.end(), pubkey_hash.begin(), pubkey_hash.end());
    script.push_back(0x88);
    script.push_back(0xAC);

    return script;
}
}; // namespace core::address
