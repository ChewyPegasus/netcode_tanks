#include "netcode/simulation.hpp"

namespace netcode {

namespace {

void clamp_to_arena(float& position, float& velocity) {
    if (position < 0.0f) {
        position = 0.0f;
        velocity = 0.0f;
    } else if (position > kArenaSize) {
        position = kArenaSize;
        velocity = 0.0f;
    }
}

}  // namespace

PlayerState step(const PlayerState& state, const PlayerInput& input, float dt) {
    Vec2 wish{static_cast<float>(input.move_x), static_cast<float>(input.move_y)};
    const float wish_length = wish.length();
    if (wish_length > 1.0f) {
        wish = wish * (1.0f / wish_length);
    }

    PlayerState next = state;
    next.velocity += wish * (kAcceleration * dt);
    next.velocity = next.velocity * (1.0f / (1.0f + kFriction * dt));

    const float speed = next.velocity.length();
    if (speed > kMaxSpeed) {
        next.velocity = next.velocity * (kMaxSpeed / speed);
    }

    next.position += next.velocity * dt;
    clamp_to_arena(next.position.x, next.velocity.x);
    clamp_to_arena(next.position.y, next.velocity.y);
    return next;
}

}  // namespace netcode
