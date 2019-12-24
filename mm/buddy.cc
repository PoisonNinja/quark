#include <arch/mm/layout.h>
#include <kernel.h>
#include <lib/bitset.h>
#include <lib/string.h>
#include <mm/buddy.h>
#include <mm/virtual.h>

namespace memory
{
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

struct buddy_order {
    stack* free_stack;
    libcxx::bitset* bitset;
};

buddy::buddy(size_t s, size_t min, size_t max)
{
    // TODO: Perform sanity checks
    this->size       = s;
    this->total_free = 0;
    this->min_order  = min;
    this->max_order  = max;
    this->orders     = new buddy_order[max + 1];
    for (size_t i = min; i <= max; i++) {
        this->orders[i].free_stack = new stack();
        this->orders[i].bitset =
            new libcxx::bitset(this->size / libcxx::pow2(i), true);
    }
}

buddy::~buddy()
{
}

addr_t buddy::__alloc(unsigned order)
{
    if (order > this->max_order)
        kernel::panic("OOM\n");
    if (this->orders[order].free_stack->empty()) {
        addr_t a = this->__alloc(order + 1);
        this->orders[order].free_stack->push(a);
        this->orders[order].free_stack->push(a + libcxx::pow2(order));
    }
    addr_t addr = this->orders[order].free_stack->pop();
    this->orders[order].bitset->set(buddy_index(addr, order));
    return addr;
}

addr_t buddy::alloc(size_t size)
{
    unsigned order = libcxx::log2(size);
    if (order < this->min_order) {
        order = this->min_order;
    }
    if (order > this->max_order) {
        kernel::panic("Unable to satisfy such a large request\n");
    }
    addr_t addr = this->__alloc(order);
    total_free -= libcxx::pow2(order);
    return addr;
}

void buddy::__free(addr_t addr, unsigned order)
{
    if (order > this->max_order) {
        return;
    }
    this->orders[order].bitset->unset(buddy_index(addr, order));
    addr_t buddy_addr = buddy_address(addr, order);
    if (!this->orders[order].bitset->test(buddy_index(buddy_addr, order))) {
        this->orders[order].free_stack->remove(buddy_addr);
        this->__free((addr < buddy_addr) ? addr : buddy_addr, order + 1);
    } else {
        this->orders[order].free_stack->push(addr);
    }
}

void buddy::free(addr_t addr, size_t size)
{
    unsigned order = libcxx::log2(size);
    if (order < this->min_order) {
        order = this->min_order;
    }
    if (order > this->max_order) {
        kernel::panic("Unable to free such a large block of memory\n");
    }
    this->__free(addr, order);
    total_free += libcxx::pow2(order);
}

bool buddy::available(size_t size)
{
    uint32_t order = libcxx::log2(size);
    for (; order <= this->max_order; order++) {
        if (!this->orders[order].free_stack->empty()) {
            return true;
        }
    }
    return false;
}

size_t buddy::total()
{
    return total_free;
}
} // namespace memory
