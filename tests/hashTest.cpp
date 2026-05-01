#include "../src/core/Sha256.hpp"
#include "../src/core/OpensslHasher.hpp"
#include "../src/core/Endian.hpp"
#include <gtest/gtest.h>

TEST(Sha256Test, TypeSizeSampleTest) {
    Sha256Hash a{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ASSERT_EQ(sizeof(a), 32);
}

TEST(Sha256Test, FromHexTest) {
    constexpr std::string_view HEX{
      "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    };
    Sha256Hash expected{
      0x00000000,
      0x0019D668,
      0x9C085AE1,
      0x65831E93,
      0x4FF763AE,
      0x46A2A6C1,
      0x72B3F1B6,
      0x0A8CE26F
    };
    Sha256Hash result = Sha256Hash::from_hex(HEX);
    ASSERT_EQ(result, expected);
}

TEST(Sha256Test, ToHexTest) {
    Sha256Hash hash{
      0x00000000,
      0x0019D668,
      0x9C085AE1,
      0x65831E93,
      0x4FF763AE,
      0x46A2A6C1,
      0x72B3F1B6,
      0x0A8CE26F
    };
    std::string expected{
      "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    };
    std::string result = hash.to_hex();
    ASSERT_EQ(result, expected);
}

TEST(Sha256Test, HexRoundtripTest) {
    constexpr std::string_view HEX{
      "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    };
    Sha256Hash hash = Sha256Hash::from_hex(HEX);
    ASSERT_EQ(hash.to_hex(), HEX);
}

TEST(Sha256Test, HashTest) {
    Sha256Hash hash = Sha256Hash::from_hex(
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
    );
    Sha256Hash expected = Sha256Hash::from_hex(
        "5df6e0e2761359d30a8275058e299fcc0381534545f55cf43e41983f5d4c9456"
    );
    OpensslHasher hasher;
    auto input_bytes = endian::to_be_bytes(hash.data);
    Sha256Hash result = hasher.hash_bytes(input_bytes);
    ASSERT_EQ(result, expected);
}

TEST(Sha256Test, ToInternalBytesGenesisHash) {
    Sha256Hash hash = Sha256Hash::from_hex(
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    );
    auto bytes = hash.to_internal_bytes();

    std::array<uint8_t, 32> expected = {
        0x6f, 0xe2, 0x8c, 0x0a, 0xb6, 0xf1, 0xb3, 0x72,
        0xc1, 0xa6, 0xa2, 0x46, 0xae, 0x63, 0xf7, 0x4f,
        0x93, 0x1e, 0x83, 0x65, 0xe1, 0x5a, 0x08, 0x9c,
        0x68, 0xd6, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    ASSERT_EQ(bytes, expected);
}

TEST(Sha256Test, FromInternalBytesGenesisHash) {
    std::array<uint8_t, 32> internal = {
        0x6f, 0xe2, 0x8c, 0x0a, 0xb6, 0xf1, 0xb3, 0x72,
        0xc1, 0xa6, 0xa2, 0x46, 0xae, 0x63, 0xf7, 0x4f,
        0x93, 0x1e, 0x83, 0x65, 0xe1, 0x5a, 0x08, 0x9c,
        0x68, 0xd6, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    Sha256Hash hash = Sha256Hash::from_internal_bytes(internal);

    Sha256Hash expected = Sha256Hash::from_hex(
        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
    );
    ASSERT_EQ(hash, expected);
}

TEST(Sha256Test, InternalBytesRoundtrip) {
    constexpr std::string_view HEX{
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    };
    Sha256Hash original = Sha256Hash::from_hex(HEX);
    auto bytes = original.to_internal_bytes();
    Sha256Hash restored = Sha256Hash::from_internal_bytes(bytes);
    ASSERT_EQ(restored, original);
    ASSERT_EQ(restored.to_hex(), HEX);
}

TEST(Sha256Test, InternalBytesAllZeros) {
    Sha256Hash zero{};
    auto bytes = zero.to_internal_bytes();
    ASSERT_EQ(bytes, (std::array<uint8_t, 32>{}));

    Sha256Hash restored = Sha256Hash::from_internal_bytes(bytes);
    ASSERT_EQ(restored, zero);
}

TEST(Sha256Test, FromHexRejectsWrongLength) {
    EXPECT_THROW(Sha256Hash::from_hex(""), std::invalid_argument);
    EXPECT_THROW(Sha256Hash::from_hex("00"), std::invalid_argument);
    EXPECT_THROW(
        Sha256Hash::from_hex(
            "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26"
        ),
        std::invalid_argument
    );
    EXPECT_THROW(
        Sha256Hash::from_hex(
            "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f00"
        ),
        std::invalid_argument
    );
}

TEST(Sha256Test, FromHexRejectsInvalidChars) {
    EXPECT_THROW(
        Sha256Hash::from_hex(
            "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26g"
        ),
        std::invalid_argument
    );
    EXPECT_THROW(
        Sha256Hash::from_hex(
            "000000000019d6689c085ae16G831e934ff763ae46a2a6c172b3f1b60a8ce26f"
        ),
        std::invalid_argument
    );
}
