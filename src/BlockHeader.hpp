#pragma once

#include <cstdint>
#include "Sha256.hpp"

struct BlockHeader {
    uint32_t version{1};
    Sha256Hash previous_hash;
    Sha256Hash merkle_root;
    uint32_t time{0};
    uint32_t bits{0};
    uint32_t nonce{0};

    [[nodiscard]] auto hash() const -> Sha256Hash;

    [[nodiscard]] auto serialize() const -> std::array<uint8_t, 80>;

    // https://learnmeabitcoin.com/technical/block/bits/
    static constexpr auto get_target(uint32_t bits) -> std::array<uint32_t, 8> {
    std::array<uint32_t, 8> target{0};

    uint32_t exponent = bits >> 24;
    uint32_t coefficient = bits & 0xFFFFFF;

    if (coefficient & 0x00800000 || coefficient == 0) {
        return target; // invalid or zero target
    }

    if (exponent <= 3) {
        coefficient >>= 8 * (3 - exponent);
        target[0] = coefficient;
    } else {
        uint32_t shift_bytes = exponent - 3;
        uint32_t shift_words = shift_bytes / 4;
        uint32_t shift_bits = (shift_bytes % 4) * 8;

        if (shift_words < 8) {
            target.at(shift_words) |= coefficient << shift_bits;
        }

        if (shift_words + 1 < 8 && shift_bits > 0) {
            target.at(shift_words + 1) |= coefficient >> (32 - shift_bits);
        }
    }
    
    return target;
}
};

struct Block {
    BlockHeader header;
};