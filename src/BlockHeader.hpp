#pragma once
#include <array>
#include <cstdint>

using Sha256Hash = std::array<uint32_t, 8>;
static_assert(sizeof(Sha256Hash) == 32, "Sha256Hash must be 32 bytes");

struct BlockHeader {
    uint32_t version;
    Sha256Hash previous_hash;
    Sha256Hash merkle_root;
    uint32_t time;
    uint32_t bits;
    uint32_t nonce;
};

struct Block {
    BlockHeader header;
};