#include "MerkleTree.hpp"
#include "Sha256.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <string_view>
#include <vector>
#include <ranges>

MerkleTree::MerkleTree(std::vector<std::string_view> txid_array) {
    namespace rv = std::views;

    txids = txid_array
        | rv::transform(Sha256Hash::from_hex)
        | std::ranges::to<std::vector<TXID>>();
}

auto MerkleTree::finalize(Hasher *hasher) const -> Sha256Hash {
    assert(!this->txids.empty());

    auto level = txids;

    while (level.size() > 1) {
        if (level.size() % 2 == 1) {
            level.push_back(level.back());
        }

        std::vector<TXID> next{};

        for (auto it = level.begin(); it != level.end(); std::advance(it, 2)) {
            auto left = it->to_internal_bytes();
            auto right = std::next(it)->to_internal_bytes();

            std::array<uint8_t, left.size() + right.size()> arr{};
            std::ranges::copy(left, arr.begin());
            std::ranges::copy(right, arr.begin() + left.size());

            next.push_back(hasher->double_hash_bytes(arr));
        }

        level = next;
    }

    return Sha256Hash{level.front()};
}