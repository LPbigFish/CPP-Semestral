#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <format>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace core {
// https://stackoverflow.com/questions/216823/how-can-i-trim-a-stdstring#217605
constexpr auto ltrim_slide(std::string_view str) -> const char* {
    return std::ranges::find_if(str, [](unsigned char c) -> bool {
        return !std::isspace(c);
    });
}

constexpr auto rtrim_slide(std::string_view str) -> const char* {
    return std::ranges::find_if(
               str | std::views::reverse,
               [](unsigned char c) -> bool { return !std::isspace(c); }
    ).base();
}

constexpr auto bytes_to_hex(std::span<const uint8_t> data) -> std::string {
    std::string hex;

    for (uint8_t byte : data) {
        std::format_to(std::back_inserter(hex), "{:02x}", byte);
    }

    return hex;
}

// NOLINTNEXTLINE(readability-identifier-naming)
template<class... Ts> struct overloaded: Ts... {
    using Ts::operator()...;
};

template<std::ranges::input_range... Ranges>
static auto concat_all(std::vector<uint8_t>& output, Ranges&&... ranges)
    -> void {
    (...,
     (std::ranges::copy(
         // Clang-tidy řekl že forwardovat
         std::forward<Ranges>(ranges),
         std::back_inserter(output)
     )));
}
}; // namespace core
