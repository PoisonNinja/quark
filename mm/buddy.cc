#include <arch/mm/layout.h>
#include <kernel.h>
#include <lib/bitset.h>
#include <lib/string.h>
#include <mm/buddy.h>
#include <mm/virtual.h>

namespace
{
constexpr addr_t buddy_address(addr_t x, unsigned int order)
{
    return ((x) ^ (1 << (order)));
}
constexpr addr_t buddy_index(addr_t x, int order)
{
    return ((x) / (libcxx::pow2(order)));
}
} // namespace

struct BuddyOrder {
    Stack* stack;
    libcxx::bitset* bitset;
};

Buddy::Buddy(size_t s, size_t min, size_t max)
{
    // TODO: Perform sanity checks
    this->size          = s;
    this->min_order     = min;
    this->max_order     = max;
    this->orders        = new BuddyOrder[max + 1];
    size_t stack_offset = 0;
    for (size_t i = min; i <= max; i++) {
        this->orders[i].stack =
            new Stack(reinterpret_cast<addr_t*>(STACK_START + stack_offset));
        stack_offset += Memory::Virtual::align_up(
            stack_overhead(this->size, libcxx::pow2(i)));
        this->orders[i].bitset =
            new libcxx::bitset(this->size / libcxx::pow2(i), true);
    }
}

Buddy::~Buddy()
{
}

addr_t Buddy::alloc(size_t size)
{
    size_t order = libcxx::log2(size);
    if (order < this->min_order)
        order = this->min_order;
    if (order > this->max_order)
        return 0;
    size_t original_order = order;
    if (this->orders[order].stack->empty()) {
        bool found = false;
        while (order++ <= this->max_order) {
            if (!this->orders[order].stack->empty()) {
                found = true;
                break;
            }
        }
        if (!found) {
            return 0;
        }
        addr_t addr = this->orders[order].stack->pop();
        for (; order > original_order; order--) {
            this->orders[order].bitset->set(buddy_index(addr, order));
            this->orders[order - 1].bitset->set(buddy_index(addr, order - 1));
            this->orders[order - 1].stack->push(buddy_address(addr, order - 1));
        }
        return addr;
    } else {
        addr_t addr = this->orders[order].stack->pop();
        this->orders[order].bitset->set(buddy_index(addr, order));
        return addr;
    }
}

void Buddy::free(addr_t addr, size_t size)
{
    uint32_t order = libcxx::log2(size);
    for (; order <= this->max_order; order++) {
        this->orders[order].bitset->unset(buddy_index(addr, order));
        addr_t buddy_addr = buddy_address(addr, order);
        if (this->orders[order].bitset->test(buddy_index(buddy_addr, order)) ||
            order == this->max_order) {
            this->orders[order].stack->push(addr);
            break;
        }
    }
}

bool Buddy::available(size_t size)
{
    uint32_t order = libcxx::log2(size);
    for (; order <= this->max_order; order++) {
        if (!this->orders[order].stack->empty()) {
            return true;
        }
    }
    return false;
}
