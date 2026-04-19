#pragma once
#include <array>
#include <cstdint>

using Sha256HashRepr = std::array<uint8_t, 32>;

struct BlockHeader {
    uint32_t version;
    Sha256HashRepr previous_hash;
    Sha256HashRepr merkle_root;
    uint32_t time;
    uint32_t bits;
    uint32_t nonce;
};