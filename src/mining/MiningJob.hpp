#pragma once

#include "../core/BlockHeader.hpp"
#include <cstdint>

struct MiningJob {
    BlockHeader block_template;
    Sha256Hash target;
    // For stratum if I implement it
    uint64_t job_id{0};
};
