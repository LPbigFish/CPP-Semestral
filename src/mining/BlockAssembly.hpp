#pragma once

#include "../networking/MiningProtocol.hpp"
#include "MiningJob.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace block_assembly {

struct Result {
    MiningJob job;
    std::vector<uint8_t> segwit_coinbase;
    std::vector<TransactionData> transactions;
    uint64_t reward;
    uint64_t height;
};

auto assemble(const BlockTemplate& tmpl) -> Result;

auto build_submission(const BlockHeader& solved_header, const Result& assembled)
    -> std::string;

} // namespace block_assembly
