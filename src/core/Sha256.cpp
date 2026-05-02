#include "Sha256.hpp"
#include "Endian.hpp"
#include <array>
#include <charconv>
#include <cstdint>
#include <format>

// https://en.cppreference.com/cpp/utility/from_chars
auto Sha256Hash::from_hex(std::string_view hex) -> Sha256Hash {
    if (hex.size() != 64) {
        throw std::invalid_argument("Hex string must be exactly 64 characters");
    }

    std::array<uint32_t, 8> result{};

    for (size_t i = 0; i < 64; i += 8) {
        uint32_t word{};
        const auto* first = hex.data() + i;
        const auto* last = hex.data() + i + 8;

        auto [end, ec] = std::from_chars(first, last, word, 16);
        if (ec != std::errc{} || end != last) {
            throw std::invalid_argument("Invalid hex string");
        }
        result.at(i / 8) = word;
    }

    return Sha256Hash{result};
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
