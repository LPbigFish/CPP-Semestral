#pragma once

#include "utils.hpp"
#include <array>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

// https://github.com/bitcoin/bitcoin/blob/master/src/base58.cpp
namespace core::base58 {

static const std::string_view psz_base58
    = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static const std::array<int8_t, 256> map_base58 = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,
  8,  -1, -1, -1, -1, -1, -1, -1, 9,  10, 11, 12, 13, 14, 15, 16, -1, 17, 18,
  19, 20, 21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1,
  -1, -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, 48,
  49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1,
};
static_assert(std::size(map_base58) == 256);

constexpr static auto decode_base58(const std::string_view& s)
    -> std::optional<std::vector<uint8_t>> {
    // triming
    const char* begin{core::ltrim_slide(s)};
    const char* end{core::rtrim_slide(s)};

    if (begin >= end) {
        return std::nullopt;
    }
    std::string_view trimmed(begin, end);

    // counting '1's, which are zeros and clean them out
    auto subview_ones = trimmed | std::views::take_while([](char c) -> bool {
                            return c == '1';
                        });
    size_t num_of_zeros = std::ranges::distance(subview_ones);
    std::string_view clean = trimmed | std::views::drop(num_of_zeros);

    // Allocation
    // Magic pomocí logaritmu v repu [Log(58) / Log(256) + 1]
    size_t size = (clean.length() * 733 / 1000) + 1;
    std::vector<unsigned char> b256(size, 0);

    int length = 0;
    for (uint8_t c : clean) {
        int32_t carry{map_base58.at(c)};

        if (carry == -1) {
            return std::nullopt;
        }

        int i{0};
        for (auto& byte : b256) {
            if (carry == 0 && i >= length) {
                break;
            }
            carry += 58 * byte;
            byte = carry % 256;
            carry /= 256;
            i++;
        }
        assert(carry == 0);
        length = i;
    }

    std::vector<uint8_t> result;
    result.reserve(num_of_zeros + length);
    result.insert(result.end(), num_of_zeros, 0x00);

    auto result_bytes = b256 | std::views::take(length) | std::views::reverse;

    for (uint8_t b : result_bytes) {
        result.push_back(b);
    }

    return result;
}

/*
constexpr auto encode_base58(std::string_view& input) ->
std::optional<std::string> {


    return "";
} */

} // namespace core::base58
