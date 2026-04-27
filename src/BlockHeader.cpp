#include "BlockHeader.hpp"
#include "Sha256.hpp"

#include <algorithm>

auto BlockHeader::hash() const -> Sha256Hash {
    std::array<uint8_t, 80> header_bytes{ this->serialize() };

    static_assert(
        sizeof(header_bytes) == 80, "Block header must be exactly 80 bytes"
    );

    auto result = Sha256Hash::hash_bytes(header_bytes).hash();
    auto result_bytes = endian::to_be_bytes(result.data);
    std::ranges::reverse(result_bytes);
    return Sha256Hash{endian::from_be_bytes<uint32_t, 8>(result_bytes)};
}

auto BlockHeader::serialize() const -> std::array<uint8_t, 80> {
    std::array<uint8_t, 80> header_bytes{};
    auto version_bytes = endian::to_le_bytes(version);
    auto prev_hash_bytes = endian::to_be_bytes(previous_hash.data);
    std::ranges::reverse(prev_hash_bytes);
    auto merkle_root_bytes = endian::to_be_bytes(merkle_root.data);
    std::ranges::reverse(merkle_root_bytes);
    auto time_bytes = endian::to_le_bytes(time);
    auto bits_bytes = endian::to_le_bytes(bits);
    auto nonce_bytes = endian::to_le_bytes(nonce);
    std::ranges::copy(version_bytes, header_bytes.begin());
    std::ranges::copy(prev_hash_bytes, header_bytes.begin() + 4);
    std::ranges::copy(merkle_root_bytes, header_bytes.begin() + 36);
    std::ranges::copy(time_bytes, header_bytes.begin() + 68);
    std::ranges::copy(bits_bytes, header_bytes.begin() + 72);
    std::ranges::copy(nonce_bytes, header_bytes.begin() + 76);

    return header_bytes;
}