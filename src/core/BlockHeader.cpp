#include "BlockHeader.hpp"
#include "Hasher.hpp"
#include "Sha256.hpp"

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <span>

auto BlockHeader::hash(const Hasher& hasher) -> Sha256Hash {
    std::array<uint8_t, 80> header_bytes{ this->serialize() };

    static_assert(
        sizeof(header_bytes) == 80, "Block header must be exactly 80 bytes"
    );

    return hasher.double_hash_bytes(header_bytes);
}

auto BlockHeader::serialize() -> std::array<uint8_t, 80> {
    std::array<uint8_t, 80> header_bytes{};
    auto version_bytes = endian::to_le_bytes(version);
    auto prev_hash_bytes = previous_hash.to_internal_bytes();
    auto merkle_root_bytes = merkle_root.to_internal_bytes();
    auto time_bytes = endian::to_le_bytes(time);
    auto bits_bytes = endian::to_le_bytes(bits);
    auto nonce_bytes = endian::to_le_bytes(nonce);

    BlockHeader::formalize(
        header_bytes,
        version_bytes, prev_hash_bytes, merkle_root_bytes, time_bytes, bits_bytes, nonce_bytes
    );

    return header_bytes;
}

// https://www.youtube.com/watch?v=v5tLFRfktWA
template <std::ranges::input_range... Ranges>
auto BlockHeader::formalize(std::span<uint8_t> output, Ranges&... ranges) -> void {
    auto iter = output.begin();
    (..., (iter = std::ranges::copy(ranges, iter).out));
}