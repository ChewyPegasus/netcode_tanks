#pragma once

#include <cstdint>

namespace netcode {

inline constexpr bool sequence_greater_than(uint16_t a, uint16_t b) {
    return ((a > b) && (a - b <= 32768)) || ((a < b) && (b - a > 32768));
}

inline constexpr bool sequence_less_than(uint16_t a, uint16_t b) { return sequence_greater_than(b, a); }

}  // namespace netcode
