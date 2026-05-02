#include "OpensslHasher.hpp"
#include "Sha256.hpp"
#include <openssl/evp.h>
#include <openssl/sha.h>

OpensslHasher::OpensslHasher() {
    if (!md_ctx) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }
    this->init();
}

auto OpensslHasher::hash_bytes(const std::span<const uint8_t>& input) const noexcept
    -> Sha256Hash {
    std::array<uint8_t, 32> hash{};
    SHA256(input.data(), input.size(), hash.data());
    return Sha256Hash::from_internal_bytes(hash);
}

auto OpensslHasher::double_hash_bytes(
    const std::span<const uint8_t>& input
) const noexcept -> Sha256Hash {
    std::array<uint8_t, 32> first_hash{};
    SHA256(input.data(), input.size(), first_hash.data());

    std::array<uint8_t, 32> final_hash{};
    SHA256(first_hash.data(), first_hash.size(), final_hash.data());

    return Sha256Hash::from_internal_bytes(final_hash);
}

auto OpensslHasher::update(const std::span<const uint8_t>& input) -> void {
    if (EVP_DigestUpdate(md_ctx.get(), input.data(), input.size()) != 1) {
        throw std::runtime_error("Failed to update digest");
    }
}

auto OpensslHasher::finalize() -> Sha256Hash {
    std::array<uint8_t, 32> hash{};
    if (EVP_DigestFinal_ex(md_ctx.get(), hash.data(), nullptr) != 1) {
        throw std::runtime_error("Failed to finalize digest");
    }
    return Sha256Hash::from_internal_bytes(hash);
}

auto OpensslHasher::clone() const -> std::unique_ptr<Hasher> {
    auto new_hasher = std::make_unique<OpensslHasher>();
    if (EVP_MD_CTX_copy_ex(new_hasher->md_ctx.get(), this->md_ctx.get()) != 1) {
        throw std::runtime_error("Failed to clone EVP_MD_CTX");
    }
    return new_hasher;
}

auto OpensslHasher::reset() -> void {
    this->init();
}

auto OpensslHasher::init() -> void {
    if (EVP_DigestInit_ex(md_ctx.get(), EVP_sha256(), nullptr) != 1) {
        throw std::runtime_error("Failed to initialize SHA-256 digest");
    }
}