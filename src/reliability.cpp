#include "netcode/reliability.hpp"

#include "netcode/sequence.hpp"

namespace netcode {

namespace {

constexpr double kRttSmoothing = 0.1;
constexpr uint32_t kAckBitCount = 32;

}  // namespace

void write_header(BitWriter& writer, const PacketHeader& header) {
    writer.write_bits(header.sequence, 16);
    writer.write_bool(header.has_ack);
    if (header.has_ack) {
        writer.write_bits(header.ack, 16);
        writer.write_bits(header.ack_bits, 32);
    }
}

bool read_header(BitReader& reader, PacketHeader& header) {
    uint32_t sequence = 0;
    if (!reader.read_bits(sequence, 16) || !reader.read_bool(header.has_ack)) {
        return false;
    }
    header.sequence = static_cast<uint16_t>(sequence);
    header.ack = 0;
    header.ack_bits = 0;
    if (header.has_ack) {
        uint32_t ack = 0;
        if (!reader.read_bits(ack, 16) || !reader.read_bits(header.ack_bits, 32)) {
            return false;
        }
        header.ack = static_cast<uint16_t>(ack);
    }
    return true;
}

PacketHeader ReliabilityEndpoint::on_send(double now_s) {
    PacketHeader header;
    header.sequence = next_sequence_;
    next_sequence_ = static_cast<uint16_t>(next_sequence_ + 1);

    header.has_ack = has_received_;
    if (has_received_) {
        header.ack = latest_received_;
        for (uint32_t i = 0; i < kAckBitCount; ++i) {
            const auto older = static_cast<uint16_t>(latest_received_ - 1 - i);
            if (was_received(older)) {
                header.ack_bits |= 1u << i;
            }
        }
    }

    sent_[header.sequence % kWindow] = SentEntry{header.sequence, now_s, true, false};
    ++stats_.sent;
    return header;
}

bool ReliabilityEndpoint::on_receive(const PacketHeader& header, double now_s) {
    if (has_received_ && sequence_less_than(header.sequence, latest_received_)) {
        const auto age = static_cast<uint16_t>(latest_received_ - header.sequence);
        if (age >= kWindow) {
            ++stats_.stale;
            return false;
        }
    }
    if (was_received(header.sequence)) {
        ++stats_.duplicates;
        return false;
    }

    received_[header.sequence % kWindow] = ReceivedEntry{header.sequence, true};
    if (!has_received_ || sequence_greater_than(header.sequence, latest_received_)) {
        latest_received_ = header.sequence;
        has_received_ = true;
    }
    ++stats_.received;

    if (header.has_ack) {
        acknowledge(header.ack, now_s);
        for (uint32_t i = 0; i < kAckBitCount; ++i) {
            if (header.ack_bits & (1u << i)) {
                acknowledge(static_cast<uint16_t>(header.ack - 1 - i), now_s);
            }
        }
    }
    return true;
}

bool ReliabilityEndpoint::is_acked(uint16_t sequence) const {
    const SentEntry& entry = sent_[sequence % kWindow];
    return entry.used && entry.sequence == sequence && entry.acked;
}

bool ReliabilityEndpoint::was_received(uint16_t sequence) const {
    const ReceivedEntry& entry = received_[sequence % kWindow];
    return entry.used && entry.sequence == sequence;
}

void ReliabilityEndpoint::acknowledge(uint16_t sequence, double now_s) {
    SentEntry& entry = sent_[sequence % kWindow];
    if (!entry.used || entry.sequence != sequence || entry.acked) {
        return;
    }
    entry.acked = true;
    ++stats_.acked;

    const double sample_ms = (now_s - entry.send_time) * 1000.0;
    if (!has_rtt_) {
        rtt_ms_ = sample_ms;
        has_rtt_ = true;
    } else {
        rtt_ms_ += (sample_ms - rtt_ms_) * kRttSmoothing;
    }
}

}  // namespace netcode
