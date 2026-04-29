#include "../src/BlockHeader.hpp"
#include <gtest/gtest.h>
#include <sys/types.h>

TEST(BlockHeaderTest, Serialization) {
    BlockHeader header;
    header.version = 1;
    header.previous_hash = Sha256Hash{0, 0, 0, 0, 0, 0, 0, 0};
    header.merkle_root = Sha256Hash::from_hex(
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    );
    header.time = 1231006505;
    header.bits = 0x1D00FFFF;
    header.nonce = 0x7C2BAC1D;

    auto serialized = header.serialize();

    std::array<uint8_t, 80> data{
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x3B, 0xA3, 0xED, 0xFD, 0x7A, 0x7B, 0x12, 0xB2, 0x7A, 0xC7, 0x2C, 0x3E,
      0x67, 0x76, 0x8F, 0x61, 0x7F, 0xC8, 0x1B, 0xC3, 0x88, 0x8A, 0x51, 0x32,
      0x3A, 0x9F, 0xB8, 0xAA, 0x4B, 0x1E, 0x5E, 0x4A, 0x29, 0xAB, 0x5F, 0x49,
      0xFF, 0xFF, 0x00, 0x1D, 0x1D, 0xAC, 0x2B, 0x7C
    };

    ASSERT_EQ(serialized, data);
}

TEST(BlockHeaderTest, DefaultConstructor) {
    BlockHeader header;
    ASSERT_EQ(header.version, 1);
    ASSERT_EQ(header.previous_hash, Sha256Hash{});
    ASSERT_EQ(header.merkle_root, Sha256Hash{});
    ASSERT_EQ(header.time, 0);
    ASSERT_EQ(header.bits, 0);
    ASSERT_EQ(header.nonce, 0);
}

TEST(BlockHeaderTest, GenesisBlockHash) {
    // Raw genesis block header:
    // 0100000000000000000000000000000000000000000000000000000000000000000000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49ffff001d1dac2b7c
    BlockHeader genesis_header;
    genesis_header.version = 1;
    genesis_header.previous_hash = Sha256Hash{0, 0, 0, 0, 0, 0, 0, 0};
    genesis_header.merkle_root = Sha256Hash::from_hex(
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    );
    genesis_header.time = 1231006505;
    genesis_header.bits = 0x1D00FFFF;
    genesis_header.nonce = 0x7C2BAC1D;

    Sha256Hash expected_hash = Sha256Hash::from_hex(
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    );

    ASSERT_EQ(genesis_header.hash(), expected_hash);
}

TEST(BlockHeaderTest, Block2Hash) {
    // Raw block 2 header:
    // 010000006fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000982051fd1e4ba744bbbe680e1fee14677ba1a3c3540bf7b1cdb606e857233e0e61bc6649ffff001d01e36299
    BlockHeader block2_header;
    block2_header.version = 1;
    block2_header.previous_hash = Sha256Hash::from_hex(
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    );
    block2_header.merkle_root = Sha256Hash::from_hex(
        "0e3e2357e806b6cdb1f70b54c3a3a17b6714ee1f0e68bebb44a74b1efd512098"
    );
    block2_header.time = 1231469665;
    block2_header.bits = 0x1D00FFFF;
    block2_header.nonce = 2573394689;

    Sha256Hash expected_hash = Sha256Hash::from_hex(
        "00000000839a8e6886ab5951d76f411475428afc90947ee320161bbf18eb6048"
    );

    ASSERT_EQ(block2_header.hash(), expected_hash);
}

TEST(BlockHeaderTest, GetTargetGenesisBits) {
    Sha256Hash target = BlockHeader::get_target(0x1D00FFFF);

    Sha256Hash genesis_hash = Sha256Hash::from_hex(
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    );

    ASSERT_TRUE(genesis_hash < target);
    ASSERT_EQ(
        target.to_hex(),
        "00000000ffff0000000000000000000000000000000000000000000000000000"
    );
}

TEST(BlockHeaderTest, GetTargetZeroBits) {
    Sha256Hash target = BlockHeader::get_target(0x01000000);
    ASSERT_EQ(target, Sha256Hash{});
}

TEST(BlockHeaderTest, GetTargetNegativeCoefficient) {
    Sha256Hash target = BlockHeader::get_target(0x01800000);
    ASSERT_EQ(target, Sha256Hash{});
}

TEST(BlockHeaderTest, GetTargetLowExponent) {
    Sha256Hash target = BlockHeader::get_target(0x03000100);
    ASSERT_EQ(target.to_hex(), "0000000000000000000000000000000000000000000000000000000000000100");
}

TEST(BlockHeaderTest, GetTargetGenesisHashBelowTarget) {
    BlockHeader genesis_header;
    genesis_header.version = 1;
    genesis_header.previous_hash = Sha256Hash{0, 0, 0, 0, 0, 0, 0, 0};
    genesis_header.merkle_root = Sha256Hash::from_hex(
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    );
    genesis_header.time = 1231006505;
    genesis_header.bits = 0x1D00FFFF;
    genesis_header.nonce = 0x7C2BAC1D;

    Sha256Hash hash = genesis_header.hash();
    Sha256Hash target = BlockHeader::get_target(genesis_header.bits);
    ASSERT_TRUE(hash < target);
}
