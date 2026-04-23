#include "../src/Sha256.hpp"
#include "../src/Endian.hpp"
#include <gtest/gtest.h>
#include <print>

TEST(Sha256Test, TypeSizeSampleTest) {
    Sha256Hash a{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ASSERT_EQ(sizeof(a), 32);
}

TEST(Sha256Test, ReverseBytesTest) {
    Sha256Hash a{0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007, 0x00000008};
    Sha256Hash expected{0x01000000, 0x02000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000, 0x08000000};
    Sha256Hash result{endian::reverse_bytes(a.data)};
    ASSERT_EQ(result, expected);
}

TEST(Sha256Test, FromHexTest) {
    constexpr std::string_view HEX{"000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"};
    Sha256Hash expected{0x00000000, 0x0019d668, 0x9c085ae1, 0x65831e93, 0x4ff763ae, 0x46a2a6c1, 0x72b3f1b6, 0x0a8ce26f};
    expected.data = endian::reverse_bytes(expected.data);
    Sha256Hash result = Sha256Hash::from_hex(HEX);
    ASSERT_EQ(result, expected);
}

TEST(Sha256Test, ToHexTest) {
    Sha256Hash hash{0x00000000, 0x0019d668, 0x9c085ae1, 0x65831e93, 0x4ff763ae, 0x46a2a6c1, 0x72b3f1b6, 0x0a8ce26f};
    std::string expected{"000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"};
    std::string result = hash.to_hex();
    ASSERT_EQ(result, expected);
}

TEST(Sha256Test, HashTest) {
    Sha256Hash hash = Sha256Hash::from_hex("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    Sha256Hash expected = Sha256Hash::from_hex("5df6e0e2761359d30a8275058e299fcc0381534545f55cf43e41983f5d4c9456");
    Sha256Hash result = hash.hash();

    std::println("Hash: {}", result.to_hex());
    std::println("Expected: {}", expected.to_hex());

    ASSERT_EQ(result, expected);
}