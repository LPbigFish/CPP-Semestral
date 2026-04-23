#pragma once

#include <array>
#include <cstdint>
#include "Sha256.hpp"

struct BlockHeader {
    uint32_t version{1};
    Sha256Hash previous_hash;
    Sha256Hash merkle_root;
    uint32_t time{0};
    uint32_t bits{0};
    uint32_t nonce{0};
};

struct Block {
    BlockHeader header;
};