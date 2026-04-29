#pragma once

#include "Endian.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <openssl/sha.h>
#include <span>
#include <string>
#include <string_view>

/**
 * Represents a 256-bit hash value, typically used for SHA-256 hashes.
 * Internally stores the hash in native representation
 */
struct Sha256Hash {
    std::array<uint32_t, 8> data{};

    Sha256Hash() = default;

    explicit Sha256Hash(const std::array<uint32_t, 8>& arr): data{arr} {}

    Sha256Hash(std::initializer_list<uint32_t> init) {
        std::copy_n(
            init.begin(), std::min(init.size(), data.size()), data.begin()
        );
    }

    auto operator==(const Sha256Hash& other) const noexcept -> bool {
        return data == other.data;
    }

    [[nodiscard]] auto get_data() const noexcept -> std::span<const uint32_t> {
        return data;
    }

    auto operator==(const std::array<uint32_t, 8>& other) const -> bool {
        return data == other;
    }

    static auto from_hex(std::string_view hex) -> Sha256Hash;

    [[nodiscard]] auto to_hex() const -> std::string;

    [[nodiscard]] auto hash() const noexcept -> Sha256Hash;

    [[nodiscard]] constexpr static auto
    hash_bytes(const std::span<const uint8_t>& input) noexcept -> Sha256Hash {
        std::array<uint8_t, 32> final_hash{};

        SHA256(input.data(), input.size(), final_hash.data());

        return Sha256Hash{endian::from_be_bytes<uint32_t, 8>(final_hash)};
    }

    [[nodiscard]] auto reversed() const noexcept -> Sha256Hash;
};

static_assert(sizeof(Sha256Hash) == 32, "Sha256Hash must be 32 bytes");
