#include "netcode/packets.hpp"

#include <cassert>

namespace netcode {

namespace {

constexpr int kTypeBits = 2;
constexpr int kInputCountBits = 3;
constexpr int kEntityCountBits = 6;

void write_axis(BitWriter& writer, int8_t axis) {
    assert(axis >= -1 && axis <= 1);
    writer.write_bits(static_cast<uint32_t>(axis + 1), 2);
}

bool read_axis(BitReader& reader, int8_t& axis) {
    uint32_t raw = 0;
    if (!reader.read_bits(raw, 2) || raw > 2) {
        return false;
    }
    axis = static_cast<int8_t>(static_cast<int>(raw) - 1);
    return true;
}

void write_vec2(BitWriter& writer, Vec2 value) {
    writer.write_float(value.x);
    writer.write_float(value.y);
}

bool read_vec2(BitReader& reader, Vec2& value) { return reader.read_float(value.x) && reader.read_float(value.y); }

bool read_type(BitReader& reader, PacketType expected) {
    uint32_t type = 0;
    return reader.read_bits(type, kTypeBits) && type == static_cast<uint32_t>(expected);
}

bool fully_consumed(const BitReader& reader) { return reader.ok() && reader.bits_remaining() < 8; }

}  // namespace

std::vector<uint8_t> serialize(const InputPacket& packet) {
    assert(!packet.inputs.empty() && packet.inputs.size() <= kMaxInputsPerPacket);

    BitWriter writer;
    writer.write_bits(static_cast<uint32_t>(PacketType::Input), kTypeBits);
    write_header(writer, packet.header);
    writer.write_bits(packet.inputs.front().sequence, 32);
    writer.write_bits(static_cast<uint32_t>(packet.inputs.size() - 1), kInputCountBits);
    for (size_t i = 0; i < packet.inputs.size(); ++i) {
        const PlayerInput& input = packet.inputs[i];
        assert(input.sequence == packet.inputs.front().sequence + i);
        write_axis(writer, input.move_x);
        write_axis(writer, input.move_y);
    }
    return writer.take();
}

std::vector<uint8_t> serialize(const SnapshotPacket& packet) {
    assert(packet.entities.size() <= kMaxSnapshotEntities);

    BitWriter writer;
    writer.write_bits(static_cast<uint32_t>(PacketType::Snapshot), kTypeBits);
    write_header(writer, packet.header);
    writer.write_bits(packet.server_tick, 32);
    writer.write_bits(packet.last_processed_input, 32);
    writer.write_bits(static_cast<uint32_t>(packet.entities.size()), kEntityCountBits);
    for (const EntityState& entity : packet.entities) {
        writer.write_bits(entity.id, 16);
        write_vec2(writer, entity.state.position);
        write_vec2(writer, entity.state.velocity);
    }
    return writer.take();
}

std::optional<InputPacket> parse_input(std::span<const uint8_t> data) {
    BitReader reader(data);
    InputPacket packet;
    uint32_t first_sequence = 0;
    uint32_t count_minus_one = 0;
    if (!read_type(reader, PacketType::Input) || !read_header(reader, packet.header) ||
        !reader.read_bits(first_sequence, 32) || !reader.read_bits(count_minus_one, kInputCountBits)) {
        return std::nullopt;
    }

    packet.inputs.resize(count_minus_one + 1);
    for (uint32_t i = 0; i < packet.inputs.size(); ++i) {
        PlayerInput& input = packet.inputs[i];
        input.sequence = first_sequence + i;
        if (!read_axis(reader, input.move_x) || !read_axis(reader, input.move_y)) {
            return std::nullopt;
        }
    }
    if (!fully_consumed(reader)) {
        return std::nullopt;
    }
    return packet;
}

std::optional<SnapshotPacket> parse_snapshot(std::span<const uint8_t> data) {
    BitReader reader(data);
    SnapshotPacket packet;
    uint32_t entity_count = 0;
    if (!read_type(reader, PacketType::Snapshot) || !read_header(reader, packet.header) ||
        !reader.read_bits(packet.server_tick, 32) || !reader.read_bits(packet.last_processed_input, 32) ||
        !reader.read_bits(entity_count, kEntityCountBits) || entity_count > kMaxSnapshotEntities) {
        return std::nullopt;
    }

    packet.entities.resize(entity_count);
    for (EntityState& entity : packet.entities) {
        uint32_t id = 0;
        if (!reader.read_bits(id, 16) || !read_vec2(reader, entity.state.position) ||
            !read_vec2(reader, entity.state.velocity)) {
            return std::nullopt;
        }
        entity.id = static_cast<ClientId>(id);
    }
    if (!fully_consumed(reader)) {
        return std::nullopt;
    }
    return packet;
}

}  // namespace netcode
