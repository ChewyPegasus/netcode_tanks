#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "netcode/reliability.hpp"
#include "netcode/simulation.hpp"

namespace netcode {

using ClientId = uint16_t;

enum class PacketType : uint8_t { Input = 0, Snapshot = 1 };

// Each input packet repeats the latest inputs so a single lost packet costs nothing.
inline constexpr size_t kMaxInputsPerPacket = 8;
inline constexpr size_t kMaxSnapshotEntities = 32;

struct InputPacket {
    PacketHeader header;
    std::vector<PlayerInput> inputs;

    bool operator==(const InputPacket&) const = default;
};

struct EntityState {
    ClientId id = 0;
    PlayerState state;

    bool operator==(const EntityState&) const = default;
};

struct SnapshotPacket {
    PacketHeader header;
    uint32_t server_tick = 0;
    uint32_t last_processed_input = 0;
    std::vector<EntityState> entities;

    bool operator==(const SnapshotPacket&) const = default;
};

std::vector<uint8_t> serialize(const InputPacket& packet);
std::vector<uint8_t> serialize(const SnapshotPacket& packet);

std::optional<InputPacket> parse_input(std::span<const uint8_t> data);
std::optional<SnapshotPacket> parse_snapshot(std::span<const uint8_t> data);

}  // namespace netcode
