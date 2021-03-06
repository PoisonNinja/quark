#pragma once

#include <kernel/lock.h>
#include <lib/bitset.h>
#include <mm/stack.h>
#include <types.h>

namespace memory
{
constexpr size_t max_num_orders = 28 - 12;

struct buddy_order;

class buddy
{
public:
    buddy(size_t s, size_t min, size_t max);
    ~buddy();

    addr_t alloc(size_t size);
    void free(addr_t addr, size_t size);

    // Returns whether there is physical memory chunks available for this size
    bool available(size_t size);

    size_t total();

private:
    addr_t __alloc(unsigned order);
    void __free(addr_t addr, unsigned order);

    size_t size;
    size_t total_free;
    size_t min_order;
    size_t max_order;

    spinlock _lock;
    struct buddy_order* orders;
};
} // namespace memory
