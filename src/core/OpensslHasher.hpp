#pragma once

#include "Hasher.hpp"
#include <memory>
#include <openssl/evp.h>

class OpensslHasher: public Hasher {
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> md_ctx{
      EVP_MD_CTX_new(), &EVP_MD_CTX_free
    };

  public:
    OpensslHasher();
    [[nodiscard]] auto
    hash_bytes(const std::span<const uint8_t>& input) const noexcept
        -> Sha256Hash override;
    [[nodiscard]] auto
    double_hash_bytes(const std::span<const uint8_t>& input) const noexcept
        -> Sha256Hash override;
    [[nodiscard]] auto clone() const -> std::unique_ptr<Hasher> override;
    auto update(const std::span<const uint8_t>& input) -> void override;
    auto finalize() -> Sha256Hash override;
    auto reset() -> void override;
private:
    auto init() -> void;
};
