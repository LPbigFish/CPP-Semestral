#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <openssl/sha.h>

struct Sha256Hash {
    std::array<uint32_t, 8> data{};

    Sha256Hash() = default;

    explicit Sha256Hash(const std::array<uint32_t, 8>& arr): data{arr} {}

    Sha256Hash(std::initializer_list<uint32_t> init) {
        std::copy_n(
            init.begin(), std::min(init.size(), data.size()), data.begin()
        );
    }

    auto operator==(const Sha256Hash& other) const -> bool {
        return data == other.data;
    }

    [[nodiscard]] auto get_data() const -> std::span<const uint32_t> {
        return data;
    }

    static auto from_hex(const std::string_view& hex) -> Sha256Hash;

    [[nodiscard]] auto hash() const -> Sha256Hash;

    [[nodiscard]] auto to_hex() const -> std::string;
};

static_assert(sizeof(Sha256Hash) == 32, "Sha256Hash must be 32 bytes");
