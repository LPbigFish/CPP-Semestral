#pragma once

#include "../core/Sha256.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

constexpr std::array<uint32_t, 64> K{
  0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1,
  0x923F82A4, 0xAB1C5ED5, 0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
  0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174, 0xE49B69C1, 0xEFBE4786,
  0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
  0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147,
  0x06CA6351, 0x14292967, 0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
  0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85, 0xA2BFE8A1, 0xA81A664B,
  0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
  0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A,
  0x5B9CCA4F, 0x682E6FF3, 0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
  0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

constexpr std::array<uint32_t, 8> INITIAL_HASH{
  0x6A09E667,
  0xBB67AE85,
  0x3C6EF372,
  0xA54FF53A,
  0x510E527F,
  0x9B05688C,
  0x1F83D9AB,
  0x5BE0CD19
};

class OwnHasher {
    struct State {
        std::array<uint8_t, 64> buffer{};
        std::array<uint32_t, 8> ctx{};
        uint64_t total_len{0};
        uint8_t buffer_ptr{0};
    };

    std::optional<State> saved_state;
    std::array<uint32_t, 8> md_ctx{};
    std::array<uint8_t, 64> buffer{};
    uint8_t buffer_ptr{0};
    uint64_t total_len{0};
    bool has_saved_state{false};

  public:
    OwnHasher() {
        std::ranges::copy(INITIAL_HASH, md_ctx.begin());
    }

    static auto hash_bytes(std::span<const uint8_t> input) noexcept
        -> Sha256Hash;
    static auto double_hash_bytes(std::span<const uint8_t> input) noexcept
        -> Sha256Hash;
    auto update(std::span<const uint8_t> input) -> void;
    auto finalize() -> Sha256Hash;
    auto reset() -> void;

    auto save_state() -> void;
    auto restore_state() -> void;

  private:
    auto compress(std::span<const uint8_t, 64> block) -> void {
        std::array<uint32_t, 64> w{};
        for (size_t t = 0; t < 16; ++t) {
            w.at(t) = (static_cast<uint32_t>(block[(4 * t) + 0]) << 24)
                    | (static_cast<uint32_t>(block[(4 * t) + 1]) << 16)
                    | (static_cast<uint32_t>(block[(4 * t) + 2]) << 8)
                    | (static_cast<uint32_t>(block[(4 * t) + 3]) << 0);
        }

        for (size_t t = 16; t < 64; ++t) {
            w.at(t) = w.at(t - 16) + lower_sigma0(w.at(t - 15)) + w.at(t - 7)
                    + lower_sigma1(w.at(t - 2));
        }

        uint32_t a = md_ctx[0];
        uint32_t b = md_ctx[1];
        uint32_t c = md_ctx[2];
        uint32_t d = md_ctx[3];
        uint32_t e = md_ctx[4];
        uint32_t f = md_ctx[5];
        uint32_t g = md_ctx[6];
        uint32_t h = md_ctx[7];

        for (size_t t = 0; t < 64; ++t) {
            uint32_t t1 = h + upper_sigma1(e) + ch(e, f, g) + K.at(t) + w.at(t);
            uint32_t t2 = upper_sigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        md_ctx[0] += a;
        md_ctx[1] += b;
        md_ctx[2] += c;
        md_ctx[3] += d;
        md_ctx[4] += e;
        md_ctx[5] += f;
        md_ctx[6] += g;
        md_ctx[7] += h;
    }

    static constexpr auto ch(uint32_t x, uint32_t y, uint32_t z) -> uint32_t {
        return (x & y) ^ (~x & z);
    }

    static constexpr auto maj(uint32_t x, uint32_t y, uint32_t z) -> uint32_t {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static constexpr auto upper_sigma0(uint32_t x) -> uint32_t {
        return std::rotr(x, 2) ^ std::rotr(x, 13) ^ std::rotr(x, 22);
    }

    static constexpr auto upper_sigma1(uint32_t x) -> uint32_t {
        return std::rotr(x, 6) ^ std::rotr(x, 11) ^ std::rotr(x, 25);
    }

    static constexpr auto lower_sigma0(uint32_t x) -> uint32_t {
        return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
    }

    static constexpr auto lower_sigma1(uint32_t x) -> uint32_t {
        return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10);
    }
};
