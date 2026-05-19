#pragma once

#include "Sha256.hpp"
#include <cstddef>
#include <memory>
#include <openssl/evp.h>
#include <span>

class OpensslHasher {
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> md_ctx{
      EVP_MD_CTX_new(), &EVP_MD_CTX_free
    };
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> saved_md_ctx{
      EVP_MD_CTX_new(), &EVP_MD_CTX_free
    };
    bool has_saved_state{false};

  public:
    OpensslHasher();
    static auto hash_bytes(const std::span<const uint8_t>& input) noexcept
        -> Sha256Hash;
    static auto
    double_hash_bytes(const std::span<const uint8_t>& input) noexcept
        -> Sha256Hash;
    auto update(const std::span<const uint8_t>& input) -> void;
    auto finalize() -> Sha256Hash;
    auto reset() -> void;

    auto save_state() -> void;
    auto restore_state() -> void;

  private:
    auto init() -> void;
};
