#pragma once

#include <cstdint>

#include "netcode/math.hpp"

namespace netcode {

inline constexpr int kTickRate = 60;
inline constexpr float kTickDt = 1.0f / static_cast<float>(kTickRate);

inline constexpr float kArenaSize = 1000.0f;
inline constexpr float kMaxSpeed = 300.0f;
inline constexpr float kAcceleration = 2400.0f;
inline constexpr float kFriction = 10.0f;

struct PlayerInput {
    uint32_t sequence = 0;
    int8_t move_x = 0;
    int8_t move_y = 0;

    bool operator==(const PlayerInput&) const = default;
};

struct PlayerState {
    Vec2 position{kArenaSize / 2.0f, kArenaSize / 2.0f};
    Vec2 velocity;

    bool operator==(const PlayerState&) const = default;
};

PlayerState step(const PlayerState& state, const PlayerInput& input, float dt = kTickDt);

}  // namespace netcode
