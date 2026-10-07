#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace netcode {

class BitWriter {
public:
    void write_bits(uint32_t value, int bits);
    void write_bool(bool value);
    void write_float(float value);

    size_t bits_written() const { return bit_pos_; }
    const std::vector<uint8_t>& bytes() const { return bytes_; }
    std::vector<uint8_t> take();

private:
    std::vector<uint8_t> bytes_;
    size_t bit_pos_ = 0;
};

class BitReader {
public:
    explicit BitReader(std::span<const uint8_t> data) : data_(data) {}

    bool read_bits(uint32_t& value, int bits);
    bool read_bool(bool& value);
    bool read_float(float& value);

    bool ok() const { return ok_; }
    size_t bits_remaining() const { return data_.size() * 8 - bit_pos_; }

private:
    std::span<const uint8_t> data_;
    size_t bit_pos_ = 0;
    bool ok_ = true;
};

}  // namespace netcode
