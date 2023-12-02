#include "nandi.h"

extern uint32_t n_math_clamp_u32(uint32_t value, uint32_t min, uint32_t max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
