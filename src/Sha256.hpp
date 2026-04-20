#pragma once

#include <array>

using Sha256Hash = std::array<uint32_t, 8>;
static_assert(sizeof(Sha256Hash) == 32, "Sha256Hash must be 32 bytes");