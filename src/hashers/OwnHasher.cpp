#include "OwnHasher.hpp"
#include <algorithm>
#include <cstdint>
#include <span>

auto OwnHasher::hash_bytes(std::span<const uint8_t> input) noexcept
    -> Sha256Hash {
    OwnHasher hasher;
    hasher.update(input);
    return hasher.finalize();
}

auto OwnHasher::double_hash_bytes(std::span<const uint8_t> input) noexcept
    -> Sha256Hash {
    auto first_hash = hash_bytes(input);
    return hash_bytes(first_hash.to_internal_bytes());
}

auto OwnHasher::update(std::span<const uint8_t> input) -> void {
    total_len += input.size();

    if (buffer_ptr > 0) {
        size_t filling = 64 - buffer_ptr;
        uint32_t to_copy = std::min(filling, input.size());

        std::ranges::copy(input.first(to_copy), buffer.begin() + buffer_ptr);
        input = input.subspan(to_copy);
        buffer_ptr += to_copy;
        if (buffer_ptr == 64) {
            compress(std::span<const uint8_t, 64>{buffer.data(), 64});
            buffer = {};
            buffer_ptr = 0;
        }
    }

    while (input.size() >= 64) {
        compress(std::span<const uint8_t, 64>{input.data(), 64});
        input = input.subspan(64);
    }

    if (!input.empty()) {
        std::ranges::copy(input, buffer.begin() + buffer_ptr);
        buffer_ptr += input.size();
    }
}

auto OwnHasher::finalize() -> Sha256Hash {
    buffer.at(buffer_ptr++) = 0x80;

    if (buffer_ptr > 56) {
        while (buffer_ptr < 64) {
            buffer.at(buffer_ptr++) = 0x00;
        }

        compress(std::span<const uint8_t, 64>{buffer.data(), 64});
        buffer = {};
        buffer_ptr = 0;
    }

    while (buffer_ptr < 56) {
        buffer.at(buffer_ptr++) = 0x00;
    }

    uint64_t total_bits = total_len * 8;
    for (int i = 7; i >= 0; --i) {
        buffer.at(buffer_ptr++)
            = static_cast<uint8_t>((total_bits >> (i * 8)) & 0xFF);
    }

    compress(std::span<const uint8_t, 64>{buffer.data(), 64});

    buffer = {};
    buffer_ptr = 0;

    return Sha256Hash::from_internal_bytes(endian::to_be_bytes(md_ctx));
}

auto OwnHasher::reset() -> void {
    std::ranges::copy(INITIAL_HASH, md_ctx.begin());
    if (saved_state) {
        std::ranges::copy(INITIAL_HASH, saved_state->ctx.begin());
        saved_state->buffer = {};
        saved_state->total_len = 0;
    }
    buffer = {};
    buffer_ptr = 0;
    total_len = 0;
    has_saved_state = false;
}

auto OwnHasher::save_state() -> void {
    if (!has_saved_state) {
        saved_state = State{};
        std::ranges::copy(md_ctx, saved_state->ctx.begin());
        std::ranges::copy(buffer, saved_state->buffer.begin());
        saved_state->total_len = total_len;
        saved_state->buffer_ptr = buffer_ptr;
        has_saved_state = true;
    }
}

auto OwnHasher::restore_state() -> void {
    if (has_saved_state) {
        std::ranges::copy(saved_state->ctx, md_ctx.begin());
        std::ranges::copy(saved_state->buffer, buffer.begin());
        total_len = saved_state->total_len;
        buffer_ptr = saved_state->buffer_ptr;
    }
}
