#include "../src/BlockHeader.hpp"
#include <gtest/gtest.h>

TEST(BlockHeaderTest, HashTest) {
    Sha256Hash a{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ASSERT_EQ(sizeof(a), 32);
}