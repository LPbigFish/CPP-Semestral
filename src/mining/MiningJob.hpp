#pragma once

#include "../core/BlockHeader.hpp"

struct MiningJob {
    BlockHeader block_header;
    Sha256Hash target;
};
