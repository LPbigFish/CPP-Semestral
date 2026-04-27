#include "../src/Sha256.hpp"
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
    Sha256Hash result = hash.hash();
    ASSERT_EQ(result, expected);
}
