#include "Sha256.hpp"
#include "Endian.hpp"
#include <array>
#include <charconv>
#include <cstdint>
#include <format>

auto Sha256Hash::from_hex(std::string_view hex) -> Sha256Hash {
    std::array<uint32_t, 8> result{};

    /* for (size_t i = 0; i < result.size() && i * 8 < hex.size(); ++i) {
        uint32_t value = 0;
        for (size_t j = 0; j < 8 && (i * 8) + j < hex.size(); ++j) {
            char c = hex[(i * 8) + j];
            value <<= 4;
            if (c >= '0' && c <= '9') {
                value |= (c - '0');
            } else if (c >= 'a' && c <= 'f') {
                value |= (c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                value |= (c - 'A' + 10);
            }
        }
        result.at(i) = value;
    } */

    for (size_t i = 0; i < 64; i += 8) {
        uint32_t word{};
        auto [end, ec]
            = std::from_chars(hex.data() + i, hex.data() + i + 8, word, 16);
        if (ec != std::errc{}) {
            throw std::invalid_argument("Invalid hex string");
        }
        result.at(i / 8) = word;
    }

    return Sha256Hash{result};
}

auto Sha256Hash::hash() const noexcept -> Sha256Hash {
    std::array<uint8_t, 32> final_hash{};

    const auto input_bytes = endian::to_be_bytes(this->data);

    SHA256(input_bytes.data(), input_bytes.size(), final_hash.data());

    return Sha256Hash{endian::from_be_bytes<uint32_t, 8>(final_hash)};
}

auto Sha256Hash::to_hex() const -> std::string {
    std::string hex;
    hex.reserve(64);

    auto bytes = endian::to_be_bytes(this->data);

    for (uint8_t byte : bytes) {
        std::format_to(std::back_inserter(hex), "{:02x}", byte);
    }

    return hex;
}

auto Sha256Hash::reversed() const noexcept -> Sha256Hash {
    auto bytes = endian::to_be_bytes(data);
    std::ranges::reverse(bytes);
    return Sha256Hash{endian::from_be_bytes<uint32_t, 8>(bytes)};
}
