#include <gtest/gtest.h>

#include "netcode/sequence.hpp"

using netcode::sequence_greater_than;
using netcode::sequence_less_than;

TEST(Sequence, ComparesNormally) {
    EXPECT_TRUE(sequence_greater_than(100, 50));
    EXPECT_FALSE(sequence_greater_than(50, 100));
    EXPECT_FALSE(sequence_greater_than(7, 7));
}

TEST(Sequence, TreatsWrappedValuesAsNewer) {
    EXPECT_TRUE(sequence_greater_than(2, 65535));
    EXPECT_FALSE(sequence_greater_than(65535, 2));
    EXPECT_TRUE(sequence_less_than(65530, 4));
}
