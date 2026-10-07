#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "netcode/bitstream.hpp"

namespace netcode {

struct PacketHeader {
    uint16_t sequence = 0;
    bool has_ack = false;
    uint16_t ack = 0;
    uint32_t ack_bits = 0;

    bool operator==(const PacketHeader&) const = default;
};

void write_header(BitWriter& writer, const PacketHeader& header);
bool read_header(BitReader& reader, PacketHeader& header);

struct ReliabilityStats {
    uint64_t sent = 0;
    uint64_t received = 0;
    uint64_t acked = 0;
    uint64_t duplicates = 0;
    uint64_t stale = 0;
};

class ReliabilityEndpoint {
public:
    // 65536 is a multiple of the window, so a sequence maps to the same slot after wraparound.
    static constexpr size_t kWindow = 1024;

    PacketHeader on_send(double now_s);
    bool on_receive(const PacketHeader& header, double now_s);

    bool is_acked(uint16_t sequence) const;
    double rtt_ms() const { return rtt_ms_; }
    const ReliabilityStats& stats() const { return stats_; }

private:
    struct SentEntry {
        uint16_t sequence = 0;
        double send_time = 0.0;
        bool used = false;
        bool acked = false;
    };

    struct ReceivedEntry {
        uint16_t sequence = 0;
        bool used = false;
    };

    bool was_received(uint16_t sequence) const;
    void acknowledge(uint16_t sequence, double now_s);

    uint16_t next_sequence_ = 0;
    uint16_t latest_received_ = 0;
    bool has_received_ = false;
    std::array<SentEntry, kWindow> sent_{};
    std::array<ReceivedEntry, kWindow> received_{};
    double rtt_ms_ = 0.0;
    bool has_rtt_ = false;
    ReliabilityStats stats_;
};

}  // namespace netcode
