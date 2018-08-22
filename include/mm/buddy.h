#pragma once

#include <lib/bitset.h>
#include <mm/stack.h>
#include <types.h>

constexpr size_t max_num_orders = 28 - 12;

struct BuddyOrder;

class Buddy
{
public:
    Buddy(size_t s, size_t min, size_t max);
    ~Buddy();

    addr_t alloc(size_t size);
    void free(addr_t addr, size_t size);

    // Returns whether there is physical memory chunks available for this size
    bool available(size_t size);

private:
    size_t size;
    size_t min_order;
    size_t max_order;
    struct BuddyOrder* orders;
};