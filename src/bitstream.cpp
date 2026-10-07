#include "netcode/bitstream.hpp"

#include <bit>
#include <cassert>
#include <utility>

namespace netcode {

void BitWriter::write_bits(uint32_t value, int bits) {
    assert(bits >= 1 && bits <= 32);
    for (int i = 0; i < bits; ++i) {
        if (bit_pos_ % 8 == 0) {
            bytes_.push_back(0);
        }
        if ((value >> i) & 1u) {
            bytes_.back() = static_cast<uint8_t>(bytes_.back() | (1u << (bit_pos_ % 8)));
        }
        ++bit_pos_;
    }
}

void BitWriter::write_bool(bool value) { write_bits(value ? 1u : 0u, 1); }

void BitWriter::write_float(float value) { write_bits(std::bit_cast<uint32_t>(value), 32); }

std::vector<uint8_t> BitWriter::take() {
    bit_pos_ = 0;
    return std::exchange(bytes_, {});
}

bool BitReader::read_bits(uint32_t& value, int bits) {
    if (!ok_ || bits < 1 || bits > 32 || bits_remaining() < static_cast<size_t>(bits)) {
        ok_ = false;
        return false;
    }
    value = 0;
    for (int i = 0; i < bits; ++i) {
        const uint8_t byte = data_[bit_pos_ / 8];
        if ((byte >> (bit_pos_ % 8)) & 1u) {
            value |= 1u << i;
        }
        ++bit_pos_;
    }
    return true;
}

bool BitReader::read_bool(bool& value) {
    uint32_t bit = 0;
    if (!read_bits(bit, 1)) {
        return false;
    }
    value = bit != 0;
    return true;
}

bool BitReader::read_float(float& value) {
    uint32_t raw = 0;
    if (!read_bits(raw, 32)) {
        return false;
    }
    value = std::bit_cast<float>(raw);
    return true;
}

}  // namespace netcode
