#pragma once

#include "Sha256.hpp"
#include <array>
#include <cstdint>

class Hasher {
public:
    virtual ~Hasher() = default;
    Hasher() = default;
    Hasher(const Hasher&) = default;
    Hasher(Hasher&&) = default;
    auto operator=(const Hasher&) -> Hasher& = default;
    auto operator=(Hasher&&) -> Hasher& = default;

    [[nodiscard]] virtual auto hash_bytes(const std::span<const uint8_t>& input) const noexcept -> Sha256Hash = 0;

    [[nodiscard]] virtual auto double_hash_bytes(const std::span<const uint8_t>& input) const noexcept -> Sha256Hash = 0;

    virtual auto update(const std::span<const uint8_t>& input) -> void = 0;

    [[nodiscard]] virtual auto hash(std::span<const uint8_t, 4> nonce) -> Sha256Hash = 0;
};