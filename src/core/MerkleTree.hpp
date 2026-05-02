#pragma once

#include "Hasher.hpp"
#include "Sha256.hpp"
#include <string_view>
#include <vector>

using TXID = Sha256Hash;

class MerkleTree {
    std::vector<TXID> txids;

public:
    explicit MerkleTree(std::vector<std::string_view> txid_array);

    auto finalize(Hasher *hasher) const -> Sha256Hash;
};