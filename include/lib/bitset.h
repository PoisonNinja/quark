#pragma once

#include <lib/math.h>
#include <types.h>

constexpr size_t bitset_size_calc(size_t size)
{
    return Math::div_round_up(size, 8);
}
constexpr size_t bitset_index(size_t index)
{
    return index / 8;
}

constexpr uint8_t bitset_full = 0xFF;
constexpr uint8_t bitset_empty = 0x00;

inline void bitset_set(uint8_t* bitset, size_t index)
{
    bitset[index / 8] |= (1 << (index % 8));
}
inline void bitset_unset(uint8_t* bitset, size_t index)
{
    bitset[index / 8] &= ~(1 << (index % 8));
}
inline bool bitset_test(uint8_t* bitset, size_t index)
{
    return (bitset[index / 8] & (1 << (index % 8))) ? true : false;
}
