#include <gtest/gtest.h>

#include <cstdint>

#include "netcode/simulation.hpp"

using netcode::kMaxSpeed;
using netcode::PlayerInput;
using netcode::PlayerState;
using netcode::step;

TEST(Simulation, StepIsDeterministic) {
    PlayerState first;
    PlayerState second;
    for (uint32_t i = 1; i <= 120; ++i) {
        const PlayerInput input{i, static_cast<int8_t>(i % 3 == 0 ? -1 : 1), 1};
        first = step(first, input);
        second = step(second, input);
    }
    EXPECT_EQ(first, second);
}

TEST(Simulation, DiagonalMovementIsNotFaster) {
    PlayerState straight;
    PlayerState diagonal;
    for (uint32_t i = 1; i <= 30; ++i) {
        straight = step(straight, PlayerInput{i, 1, 0});
        diagonal = step(diagonal, PlayerInput{i, 1, 1});
    }
    EXPECT_NEAR(straight.velocity.length(), diagonal.velocity.length(), 1e-3);
}

TEST(Simulation, SpeedStaysBounded) {
    PlayerState state;
    state.position = {0.0f, 500.0f};
    for (uint32_t i = 1; i <= 120; ++i) {
        state = step(state, PlayerInput{i, 1, 0});
        EXPECT_LE(state.velocity.length(), kMaxSpeed);
    }
}

TEST(Simulation, StopsAtTheArenaEdge) {
    PlayerState state;
    for (uint32_t i = 1; i <= 600; ++i) {
        state = step(state, PlayerInput{i, -1, 0});
    }
    EXPECT_EQ(state.position.x, 0.0f);
    EXPECT_EQ(state.velocity.x, 0.0f);
}
