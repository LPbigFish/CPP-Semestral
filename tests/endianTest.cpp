#include "../src/Endian.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>

TEST(EndianTest, ToBeBytesUint32) {
    constexpr auto RESULT = endian::to_be_bytes(uint32_t{0x01020304});
    ASSERT_EQ(RESULT, (std::array<uint8_t, 4>{0x01, 0x02, 0x03, 0x04}));
}

// 2. Scalar little-endian encoding
TEST(EndianTest, ToLeBytesUint32) {
    constexpr auto RESULT = endian::to_le_bytes(uint32_t{0x01020304});
    ASSERT_EQ(RESULT, (std::array<uint8_t, 4>{0x04, 0x03, 0x02, 0x01}));
}

// 3. Scalar big-endian decode (inverse)
TEST(EndianTest, FromBeBytesUint32) {
    constexpr auto RESULT = endian::from_be_bytes<uint32_t>(
        std::array<uint8_t, 4>{0xAA, 0xBB, 0xCC, 0xDD}
    );
    ASSERT_EQ(RESULT, 0xAABBCCDD);
}

// 4. Scalar roundtrip: encode → decode is identity
TEST(EndianTest, RoundtripBeUint32) {
    constexpr uint32_t ORIGINAL = 0xDEADBEEF;
    constexpr auto ENCODED = endian::to_be_bytes(ORIGINAL);
    constexpr auto DECODED = endian::from_be_bytes<uint32_t>(ENCODED);
    ASSERT_EQ(DECODED, ORIGINAL);
}

// 5. Array big-endian encoding (produces flat byte array)
TEST(EndianTest, ToBeBytesArray) {
    constexpr std::array<uint32_t, 2> INPUT{0x01020304, 0x05060708};
    constexpr auto RESULT = endian::to_be_bytes(INPUT);
    ASSERT_EQ(
        RESULT,
        (std::array<uint8_t, 8>{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08})
    );
}

// 6. Array roundtrip: to_be_bytes_array → from_be_bytes_array is identity
TEST(EndianTest, RoundtripBeArray) {
    constexpr std::array<uint32_t, 8> ORIGINAL{
      0x00000000,
      0x0019D668,
      0x9C085AE1,
      0x65831E93,
      0x4FF763AE,
      0x46A2A6C1,
      0x72B3F1B6,
      0x0A8CE26F
    };
    constexpr auto ENCODED = endian::to_be_bytes(ORIGINAL);
    constexpr auto DECODED = endian::from_be_bytes<uint32_t, 8>(ENCODED);
    ASSERT_EQ(DECODED, ORIGINAL);
}

TEST(EndianTest, RoundtripLeArray) {
    constexpr std::array<uint32_t, 8> ORIGINAL{
      0x00000000,
      0x0019D668,
      0x9C085AE1,
      0x65831E93,
      0x4FF763AE,
      0x46A2A6C1,
      0x72B3F1B6,
      0x0A8CE26F
    };
    constexpr auto ENCODED = endian::to_le_bytes(ORIGINAL);
    constexpr auto DECODED = endian::from_le_bytes<uint32_t, 8>(ENCODED);
    ASSERT_EQ(DECODED, ORIGINAL);
}

TEST(EndianTest, FromLeBytes32) {
    constexpr auto RESULT = endian::from_le_bytes<uint32_t>(
        std::array<uint8_t, 4>{0xDD, 0xCC, 0xBB, 0xAA}
    );
    ASSERT_EQ(RESULT, 0xAABBCCDD);
}

TEST(EndianTest, RoundtripLeUint32) {
    constexpr uint32_t ORIGINAL = 0xDEADBEEF;
    constexpr auto ENCODED = endian::to_le_bytes(ORIGINAL);
    constexpr auto DECODED = endian::from_le_bytes<uint32_t>(ENCODED);
    ASSERT_EQ(DECODED, ORIGINAL);
}