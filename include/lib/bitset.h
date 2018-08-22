#pragma once

#include <lib/math.h>
#include <types.h>

class Bitset
{
public:
    Bitset(size_t size, uint8_t initial);
    Bitset(size_t size);
    ~Bitset();

    void set(size_t index);
    void unset(size_t index);
    bool test(size_t index);

private:
    size_t allocated;
    uint8_t* bitset;
};

constexpr uint8_t bitset_empty = 0x00;
constexpr uint8_t bitset_full  = 0xFF;