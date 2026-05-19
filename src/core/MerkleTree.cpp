#include "MerkleTree.hpp"
#include "OpensslHasher.hpp"
#include "Sha256.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <vector>

MerkleTree::MerkleTree(std::vector<std::string_view> txid_array) {
    namespace rv = std::views;

    txids = txid_array | rv::transform(Sha256Hash::from_hex)
          | std::ranges::to<std::vector<TXID>>();
}

auto MerkleTree::finalize() const -> Sha256Hash {
    // Should never happen
    assert(!this->txids.empty());
    if (this->txids.empty()) {
        return Sha256Hash{};
    }

    auto level = txids;

    // If one present, return it
    while (level.size() > 1) {
        if (level.size() % 2 == 1) {
            level.push_back(level.back());
        }

        std::vector<TXID> next;
        next.reserve(level.size() / 2);

        for (size_t i = 0; i < level.size() - 1; i += 2) {
            auto left = level.at(i).to_internal_bytes();
            auto right = level.at(i + 1).to_internal_bytes();

            std::array<uint8_t, left.size() + right.size()> concat{};
            std::ranges::copy(left, concat.begin());
            std::ranges::copy(right, concat.begin() + left.size());

            next.push_back(OpensslHasher::double_hash_bytes(concat));
        }

        level = std::move(next);
    }

    return level.front();
}
