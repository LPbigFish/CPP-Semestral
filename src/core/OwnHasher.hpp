#pragma once

#include "Hasher.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

class OwnHasher: public Hasher {
    std::array<uint32_t, 8> ctx;
    std::array<uint8_t, 64> buffer{};
    uint64_t total_bytes{0};
    size_t buflen{0};

  public:
    OwnHasher();
    [[nodiscard]] auto
    hash_bytes(const std::span<const uint8_t>& input) const noexcept
        -> Sha256Hash override;
    [[nodiscard]] auto
    double_hash_bytes(const std::span<const uint8_t>& input) const noexcept
        -> Sha256Hash override;
    [[nodiscard]] auto clone() const -> std::unique_ptr<Hasher> override;

    /**
    * Copy into buffer, if full, run compression, if space, wait for finalization
    */
    auto update(const std::span<const uint8_t>& input) -> void override;

    /**
    * Add b10000000 to the buffer and run the last compression
    */
    auto finalize() -> Sha256Hash override;

    auto reset() -> void override;

    auto save_state() -> void override;

    auto restore_state() -> void override;
private:
    auto init() -> void;
};