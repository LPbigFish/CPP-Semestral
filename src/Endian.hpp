#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <bit>
#include <cstddef>
#include <cstdint>

namespace endian {
/*     constexpr auto reverse_bytes(const Sha256Hash& hash) -> Sha256Hash {
        Sha256Hash result{};
        std::ranges::transform(hash, result.begin(), [](uint32_t x) { return std::byteswap(x); });
        std::ranges::reverse(result);
        return result;
    } */

    // https://en.cppreference.com/cpp/concepts/unsigned_integral
    template<std::unsigned_integral T>
    constexpr auto reverse_bytes(T value) noexcept -> T {
        return std::byteswap(value);
    }

    // https://www.geeksforgeeks.org/cpp/templates-cpp/
    template<std::unsigned_integral T, std::size_t N>
    constexpr auto reverse_bytes(const std::array<T, N>& value) noexcept -> std::array<T, N> {
        std::array<T, N> result;
        std::ranges::transform(value, result.begin(), [](T x) -> T { return std::byteswap(x); });
        return result;
    }

    template<std::unsigned_integral T>
    constexpr auto to_le_bytes(T value) noexcept -> std::array<uint8_t, sizeof(T)> {
        std::array<uint8_t, sizeof(T)> bytes{};
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            bytes[i] = static_cast<uint8_t>(value >> (i * 8));
        }
        return bytes;
    }

    template<std::unsigned_integral T>
    constexpr auto to_be_bytes(T value) noexcept -> std::array<uint8_t, sizeof(T)> {
        std::array<uint8_t, sizeof(T)> bytes{};
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            bytes[sizeof(T) - 1 - i] = static_cast<uint8_t>(value >> (i * 8));
        }
        return bytes;
    }

    template<std::unsigned_integral T>
    constexpr auto from_le_bytes(const std::array<uint8_t, sizeof(T)>& bytes) noexcept -> T {
        T value = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            value |= static_cast<T>(bytes[i]) << (i * 8);
        }
        return value;
    }

    template<std::unsigned_integral T>
    constexpr auto from_be_bytes(const std::array<uint8_t, sizeof(T)>& bytes) noexcept -> T {
        T value = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            value |= static_cast<T>(bytes[sizeof(T) - 1 - i]) << (i * 8);
        }
        return value;
    }

    template<std::unsigned_integral T, std::size_t N>
    constexpr auto to_le_bytes(const std::array<T, N>& value) noexcept -> std::array<uint8_t, sizeof(T) * N> {
        std::array<uint8_t, sizeof(T) * N> bytes{};
        for (std::size_t i = 0; i < N; ++i) {
            auto element_bytes = to_le_bytes(value[i]);
            std::copy(element_bytes.begin(), element_bytes.end(), bytes.begin() + (i * sizeof(T)));
        }
        return bytes;
    }

    template<std::unsigned_integral T, std::size_t N>
    constexpr auto to_be_bytes(const std::array<T, N>& value) noexcept -> std::array<uint8_t, sizeof(T) * N> {
        std::array<uint8_t, sizeof(T) * N> bytes{};

        for (std::size_t i = 0; i < N; ++i) {
            auto element_bytes = to_be_bytes(value[i]);
            std::copy(element_bytes.begin(), element_bytes.end(), bytes.begin() + (i * sizeof(T)));
        }
        return bytes;
    }

    template<std::unsigned_integral T, std::size_t N>
    constexpr auto from_le_bytes(const std::array<uint8_t, sizeof(T) * N>& bytes) noexcept -> std::array<T, N> {
        std::array<T, N> value{};
        for (std::size_t i = 0; i < N; ++i) {
            std::array<uint8_t, sizeof(T)> element_bytes{};
            std::copy(bytes.begin() + (i * sizeof(T)), bytes.begin() + ((i + 1) * sizeof(T)), element_bytes.begin());
            value[i] = from_le_bytes<T>(element_bytes);
        }
        return value;
    }

    template<std::unsigned_integral T, std::size_t N>
    constexpr auto from_be_bytes(const std::array<uint8_t, sizeof(T) * N>& bytes) noexcept -> std::array<T, N> {
        std::array<T, N> value{};
        for (std::size_t i = 0; i < N; ++i) {
            std::array<uint8_t, sizeof(T)> element_bytes{};
            std::copy(bytes.begin() + (i * sizeof(T)), bytes.begin() + ((i + 1) * sizeof(T)), element_bytes.begin());
            value[i] = from_be_bytes<T>(element_bytes);
        }
        return value;
    }
}  // namespace endian