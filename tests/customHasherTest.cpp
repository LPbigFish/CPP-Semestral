#include "../src/core/OwnHasher.hpp"
#include <gtest/gtest.h>

TEST(CustomHasher, HashAHash) {
    OwnHasher oh;
    auto h1{Sha256Hash::from_hex("6b86b273ff34fce19d6b804eff5a3f5747ada4eaa22f1d49c01e52ddb7875b4b")};
    auto expected{Sha256Hash::from_hex("e0bc614e4fd035a488619799853b075143deea596c477b8dc077e309c0fe42e9")};

    auto result = oh.hash_bytes(h1.to_internal_bytes());
    ASSERT_EQ(expected, result);
}