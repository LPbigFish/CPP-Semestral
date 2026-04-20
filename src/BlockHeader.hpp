#pragma once
#include <array>
#include <cstdint>
#include "Sha256.hpp"

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