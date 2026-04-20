#pragma once
#include "Sha256.hpp"
#include <algorithm>
#include <concepts>
#include <ranges>
#include <bit>

namespace endian {
/*     constexpr auto reverse_bytes(const Sha256Hash& hash) -> Sha256Hash {
        Sha256Hash result{};
        std::ranges::transform(hash, result.begin(), [](uint32_t x) { return std::byteswap(x); });
        std::ranges::reverse(result);
        return result;
    } */

    // https://en.cppreference.com/cpp/concepts/unsigned_integral
    template<std::unsigned_integral T>
    constexpr auto reverse_bytes(T value) -> T {
        return std::byteswap(value);
    }

    // https://www.geeksforgeeks.org/cpp/templates-cpp/
    template<std::unsigned_integral T, std::size_t N>
    constexpr auto reverse_bytes(const std::array<T, N>& values) -> std::array<T, N> {
        std::array<T, N> result{};
        std::ranges::transform(values, result.begin(), [](T x) -> auto { return std::byteswap(x); });
        std::ranges::reverse(result);
        return result;
    }
}  // namespace endian