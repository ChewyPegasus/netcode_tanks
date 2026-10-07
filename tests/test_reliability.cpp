#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

#include "netcode/reliability.hpp"

using netcode::PacketHeader;
using netcode::ReliabilityEndpoint;

TEST(Reliability, AcknowledgesEveryPacketOnAPerfectLink) {
    ReliabilityEndpoint a;
    ReliabilityEndpoint b;
    double t = 0.0;
    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(b.on_receive(a.on_send(t), t + 0.05));
        ASSERT_TRUE(a.on_receive(b.on_send(t + 0.05), t + 0.1));
        t += 0.1;
    }

    EXPECT_EQ(a.stats().acked, 100u);
    // b's last packet has not been acknowledged yet because a never replied to it.
    EXPECT_EQ(b.stats().acked, 99u);
    EXPECT_NEAR(a.rtt_ms(), 100.0, 1e-6);
}

TEST(Reliability, AckBitsDescribeGaps) {
    ReliabilityEndpoint a;
    ReliabilityEndpoint b;
    std::vector<PacketHeader> sent;
    for (int i = 0; i < 5; ++i) {
        sent.push_back(a.on_send(0.0));
    }
    for (size_t index : {0u, 1u, 3u, 4u}) {
        ASSERT_TRUE(b.on_receive(sent[index], 0.01));
    }

    const PacketHeader reply = b.on_send(0.02);
    EXPECT_TRUE(reply.has_ack);
    EXPECT_EQ(reply.ack, 4);
    EXPECT_EQ(reply.ack_bits, 0b1101u);

    ASSERT_TRUE(a.on_receive(reply, 0.03));
    EXPECT_TRUE(a.is_acked(0));
    EXPECT_TRUE(a.is_acked(1));
    EXPECT_FALSE(a.is_acked(2));
    EXPECT_TRUE(a.is_acked(3));
    EXPECT_TRUE(a.is_acked(4));
    EXPECT_EQ(a.stats().acked, 4u);
}

TEST(Reliability, RejectsDuplicates) {
    ReliabilityEndpoint a;
    ReliabilityEndpoint b;
    const PacketHeader header = a.on_send(0.0);
    EXPECT_TRUE(b.on_receive(header, 0.01));
    EXPECT_FALSE(b.on_receive(header, 0.02));
    EXPECT_EQ(b.stats().duplicates, 1u);
    EXPECT_EQ(b.stats().received, 1u);
}

TEST(Reliability, KeepsWorkingAcrossSequenceWraparound) {
    ReliabilityEndpoint a;
    ReliabilityEndpoint b;
    for (int i = 0; i < 70000; ++i) {
        const double t = i * 0.001;
        ASSERT_TRUE(b.on_receive(a.on_send(t), t));
        ASSERT_TRUE(a.on_receive(b.on_send(t), t));
    }
    EXPECT_EQ(a.stats().acked, 70000u);
    EXPECT_EQ(a.stats().duplicates, 0u);
}

TEST(Reliability, HeaderRoundTrips) {
    const PacketHeader original{513, true, 512, 0xDEADBEEFu};
    netcode::BitWriter writer;
    netcode::write_header(writer, original);
    const auto bytes = writer.take();

    netcode::BitReader reader(bytes);
    PacketHeader parsed;
    ASSERT_TRUE(netcode::read_header(reader, parsed));
    EXPECT_EQ(parsed, original);
}
