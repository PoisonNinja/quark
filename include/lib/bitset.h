#pragma once

#include <types.h>

// Division, but rounding up
#define DIV_ROUND_UP(x, y) (((x) + ((y)-1)) / (y))

#define BITSET_SIZE_CALC(size) (DIV_ROUND_UP(size, 8))
#define BITSET_INDEX(index) (index / 8)

#define BITSET_FULL 0xFF
#define BITSET_EMPTY 0x00

#define bitset_set(bitset, index) bitset[index / 8] |= (1 << (index % 8))
#define bitset_unset(bitset, index) bitset[index / 8] &= ~(1 << (index % 8))
#define bitset_test(bitset, index) \
    ((bitset[index / 8] & (1 << (index % 8))) ? 1 : 0)
