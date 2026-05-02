#include "../src/core/OpensslHasher.hpp"
#include "../src/core/BlockHeader.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <span>
#include <string_view>

namespace {
auto genesis_header_bytes() -> std::array<uint8_t, 80> {
    BlockHeader genesis_header;
    genesis_header.version = 1;
    genesis_header.previous_hash = Sha256Hash{0, 0, 0, 0, 0, 0, 0, 0};
    genesis_header.merkle_root = Sha256Hash::from_hex(
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    );
    genesis_header.time = 1231006505;
    genesis_header.bits = 0x1D00FFFF;
    genesis_header.nonce = 0x7C2BAC1D;

    return genesis_header.serialize();
}
} // namespace


TEST(OpensslHasherTest, GenesisBlockDoubleHash) {
    const auto header = genesis_header_bytes();
    OpensslHasher hasher;
    Sha256Hash result = hasher.double_hash_bytes(header);
    Sha256Hash expected = Sha256Hash::from_hex(
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    );
    ASSERT_EQ(result, expected);
}

TEST(OpensslHasherTest, GenesisBlockStreamingMatchesOneShot) {
    const auto header = genesis_header_bytes();
    OpensslHasher oneshot;
    Sha256Hash oneshot_result = oneshot.hash_bytes(header);

    OpensslHasher streaming;
    streaming.update(header);
    Sha256Hash streaming_result = streaming.finalize();

    ASSERT_EQ(streaming_result, oneshot_result);
}

TEST(OpensslHasherTest, ResetAllowsReuse) {
    const auto header = genesis_header_bytes();

    OpensslHasher hasher;
    hasher.update(header);
    Sha256Hash first = hasher.finalize();

    hasher.reset();
    hasher.update(header);
    Sha256Hash second = hasher.finalize();

    ASSERT_EQ(first, second);

    OpensslHasher fresh;
    Sha256Hash expected = fresh.hash_bytes(header);
    ASSERT_EQ(first, expected);
}