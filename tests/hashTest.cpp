#include "../src/Sha256.hpp"
#include "../src/Endian.hpp"
#include <gtest/gtest.h>

TEST(Sha256Test, TypeSizeSampleTest) {
    Sha256Hash a{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ASSERT_EQ(sizeof(a), 32);
}

TEST(Sha256Test, ReverseBytesTest) {
    Sha256Hash a{0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007, 0x00000008};
    Sha256Hash expected{0x08000000, 0x07000000, 0x06000000, 0x05000000, 0x04000000, 0x03000000, 0x02000000, 0x01000000};
    Sha256Hash result = endian::reverse_bytes(a);
    ASSERT_EQ(result, expected);
}