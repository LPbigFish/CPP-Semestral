#pragma once

#include "../core/Sha256.hpp"
#include <concepts>
#include <cstdint>
#include <span>
#include <sys/types.h>

template<typename T>
concept Hasher = requires(T hasher, std::span<const uint8_t> data) {
    { hasher.hash_bytes(data) } -> std::same_as<Sha256Hash>;
    { hasher.double_hash_bytes(data) } -> std::same_as<Sha256Hash>;
    { hasher.update(data) } -> std::same_as<void>;
    { hasher.finalize() } -> std::same_as<Sha256Hash>;
    { hasher.reset() } -> std::same_as<void>;
    { hasher.save_state() } -> std::same_as<void>;
    { hasher.restore_state() } -> std::same_as<void>;
};
