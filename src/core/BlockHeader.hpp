#pragma once

#include "Hasher.hpp"
#include "Sha256.hpp"
#include <algorithm>
#include <cstdint>

struct BlockHeader {
    uint32_t version{1};
    Sha256Hash previous_hash;
    Sha256Hash merkle_root;
    uint32_t time{0};
    uint32_t bits{0};
    uint32_t nonce{0};

    [[nodiscard]] auto hash(const Hasher& hasher) -> Sha256Hash;

    [[nodiscard]] auto serialize() -> std::array<uint8_t, 80>;

    static constexpr auto get_target(uint32_t bits) -> Sha256Hash {
        std::array<uint32_t, 8> le_words{};

        uint32_t exponent = bits >> 24;
        uint32_t coefficient = bits & 0xFFFFFF;

        if (coefficient & 0x00800000 || coefficient == 0) {
            return Sha256Hash{};
        }

        if (exponent <= 3) {
            coefficient >>= 8 * (3 - exponent);
            le_words[0] = coefficient;
        } else {
            uint32_t shift_bytes = exponent - 3;
            uint32_t shift_words = shift_bytes / 4;
            uint32_t shift_bits = (shift_bytes % 4) * 8;

            if (shift_words < 8) {
                le_words.at(shift_words) |= coefficient << shift_bits;
            }

            if (shift_words + 1 < 8 && shift_bits > 0) {
                le_words.at(shift_words + 1) |= coefficient >> (32 - shift_bits);
            }
        }

        std::ranges::reverse(le_words);
        return Sha256Hash{le_words};
    }

private:
    template <std::ranges::input_range... Ranges>
    auto formalize(std::span<uint8_t> output, Ranges&... ranges) -> void;
};

struct Block {
    BlockHeader header;
};
