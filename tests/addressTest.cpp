#include "../src/core/address.hpp"
#include <gtest/gtest.h>
#include <string>
#include <string_view>

namespace a = core::address;

TEST(AddressTest, decodeAddressToScriptPubKey) {
    std::string_view expected{ "76a9146e93d7cd80b685fa255ce339282933cde8b8926988ac" };
    std::string_view input{ "mqbdjTxkpnFjxNiVqW1A5wWAjiysVsz91j" };

    auto result = a::craft_p2pkh_scriptpubkey(input);

    ASSERT_TRUE(result.has_value());

    ASSERT_EQ(core::bytes_to_hex(result.value()), expected);
}