#include <gtest/gtest.h>

#include <cstdint>
#include <span>
#include <vector>

#include "netcode/packets.hpp"

using namespace netcode;

namespace {

InputPacket make_input_packet() {
    InputPacket packet;
    packet.header = PacketHeader{42, true, 41, 0xDEADBEEFu};
    for (uint32_t i = 0; i < kMaxInputsPerPacket; ++i) {
        const auto axis = static_cast<int8_t>(static_cast<int>(i % 3) - 1);
        packet.inputs.push_back(PlayerInput{100 + i, axis, static_cast<int8_t>(-axis)});
    }
    return packet;
}

SnapshotPacket make_snapshot_packet() {
    SnapshotPacket packet;
    packet.header = PacketHeader{7, true, 6, 0x3u};
    packet.server_tick = 1234;
    packet.last_processed_input = 567;
    packet.entities.push_back(EntityState{0, PlayerState{{12.5f, 99.25f}, {-3.0f, 4.0f}}});
    packet.entities.push_back(EntityState{3, PlayerState{{0.0f, 1000.0f}, {0.0f, 0.0f}}});
    return packet;
}

}  // namespace

TEST(Packets, InputPacketRoundTrips) {
    const InputPacket original = make_input_packet();
    const auto parsed = parse_input(serialize(original));
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(*parsed, original);
}

TEST(Packets, SnapshotPacketRoundTrips) {
    const SnapshotPacket original = make_snapshot_packet();
    const auto parsed = parse_snapshot(serialize(original));
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(*parsed, original);
}

TEST(Packets, RejectsEveryTruncation) {
    const std::vector<uint8_t> bytes = serialize(make_snapshot_packet());
    for (size_t length = 0; length < bytes.size(); ++length) {
        EXPECT_FALSE(parse_snapshot(std::span<const uint8_t>(bytes.data(), length)).has_value()) << "length " << length;
    }
}

TEST(Packets, RejectsTrailingBytes) {
    std::vector<uint8_t> bytes = serialize(make_input_packet());
    bytes.push_back(0);
    EXPECT_FALSE(parse_input(bytes).has_value());
}

TEST(Packets, RejectsTheWrongPacketType) {
    EXPECT_FALSE(parse_snapshot(serialize(make_input_packet())).has_value());
    EXPECT_FALSE(parse_input(serialize(make_snapshot_packet())).has_value());
}

TEST(Packets, RejectsAnInvalidMoveValue) {
    BitWriter writer;
    writer.write_bits(static_cast<uint32_t>(PacketType::Input), 2);
    write_header(writer, PacketHeader{});
    writer.write_bits(1, 32);
    writer.write_bits(0, 3);
    writer.write_bits(3, 2);
    writer.write_bits(1, 2);
    EXPECT_FALSE(parse_input(writer.take()).has_value());
}
