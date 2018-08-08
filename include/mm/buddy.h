#pragma once

#include <mm/stack.h>
#include <types.h>

struct BuddyOrder {
    Stack free;
    uint8_t* bitset;
};

constexpr size_t max_num_orders = 28 - 12;

class Buddy
{
public:
    Buddy(size_t s, size_t min, size_t max);
    ~Buddy();

    addr_t alloc(size_t size);
    void free(addr_t addr, size_t size);

private:
    size_t size;
    size_t min_order;
    size_t max_order;
    struct BuddyOrder* orders;
};