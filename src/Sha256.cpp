#include "Sha256.hpp"
#include "Endian.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <format>

auto Sha256Hash::from_hex(const std::string_view& hex) -> Sha256Hash {
    std::array<uint32_t, 8> result{};

    for (size_t i = 0; i < result.size() && i * 8 < hex.size(); ++i) {
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
    }

    return Sha256Hash{endian::reverse_bytes(result)};
}

auto Sha256Hash::hash() const -> Sha256Hash {
    std::array<uint8_t, 32> final_hash{};
    
    const std::array<uint8_t, 32> input_bytes{std::bit_cast<std::array<uint8_t, 32>>(this->data)};

    SHA256(
        input_bytes.data(), 
        input_bytes.size(),
        final_hash.data()
    );
    
    return Sha256Hash{std::bit_cast<std::array<uint32_t, 8>>(final_hash)};
}

auto Sha256Hash::to_hex() const -> std::string {
    std::string hex;
    hex.reserve(64);

    auto bytes = std::bit_cast<std::array<uint8_t, 32>>(endian::reverse_bytes(this->data));

    for (uint8_t byte : bytes) {
        std::format_to(std::back_inserter(hex), "{:02x}", byte);
    }

    return hex;
}