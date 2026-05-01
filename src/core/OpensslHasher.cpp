#include "OpensslHasher.hpp"
#include "Endian.hpp"
#include <openssl/evp.h>
#include <openssl/sha.h>

OpensslHasher::OpensslHasher() {
    if (!md_ctx) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }
}

auto OpensslHasher::hash_bytes(const std::span<const uint8_t>& input) const noexcept
    -> Sha256Hash {
    std::array<uint8_t, 32> hash{};
    SHA256(input.data(), input.size(), hash.data());
    return Sha256Hash{endian::from_be_bytes<uint32_t, 8>(hash)};
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

auto OpensslHasher::hash(std::span<const uint8_t, 4> nonce) -> Sha256Hash {
    this->update(nonce);
    std::array<uint8_t, 32> hash{};
    EVP_DigestFinal_ex(md_ctx.get(), hash.data(), nullptr);
    return Sha256Hash::from_internal_bytes(hash);
}
