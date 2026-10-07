#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "netcode/bitstream.hpp"

using netcode::BitReader;
using netcode::BitWriter;

TEST(BitStream, RoundTripsMixedWidths) {
    BitWriter writer;
    writer.write_bits(5, 3);
    writer.write_bool(true);
    writer.write_bits(0xABCDu, 16);
    writer.write_bits(0xFFFFFFFFu, 32);
    writer.write_float(-123.456f);
    writer.write_bool(false);

    const std::vector<uint8_t> bytes = writer.take();
    BitReader reader(bytes);

    uint32_t value = 0;
    bool flag = false;
    float number = 0.0f;
    ASSERT_TRUE(reader.read_bits(value, 3));
    EXPECT_EQ(value, 5u);
    ASSERT_TRUE(reader.read_bool(flag));
    EXPECT_TRUE(flag);
    ASSERT_TRUE(reader.read_bits(value, 16));
    EXPECT_EQ(value, 0xABCDu);
    ASSERT_TRUE(reader.read_bits(value, 32));
    EXPECT_EQ(value, 0xFFFFFFFFu);
    ASSERT_TRUE(reader.read_float(number));
    EXPECT_EQ(number, -123.456f);
    ASSERT_TRUE(reader.read_bool(flag));
    EXPECT_FALSE(flag);
    EXPECT_TRUE(reader.ok());
}

TEST(BitStream, PacksBitsWithoutByteAlignment) {
    BitWriter writer;
    for (int i = 0; i < 10; ++i) {
        writer.write_bool(true);
    }
    EXPECT_EQ(writer.bits_written(), 10u);
    EXPECT_EQ(writer.bytes().size(), 2u);
}

TEST(BitStream, ReadingPastTheEndFailsAndStaysFailed) {
    const std::vector<uint8_t> bytes{0xFF};
    BitReader reader(bytes);
    uint32_t value = 0;
    EXPECT_FALSE(reader.read_bits(value, 9));
    EXPECT_FALSE(reader.ok());
    EXPECT_FALSE(reader.read_bits(value, 1));
}
