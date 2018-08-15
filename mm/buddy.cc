#include <arch/mm/layout.h>
#include <kernel.h>
#include <lib/bitset.h>
#include <lib/string.h>
#include <mm/buddy.h>
#include <mm/virtual.h>

constexpr addr_t buddy_address(addr_t x, unsigned int order)
{
    return ((x) ^ (1 << (order)));
}
constexpr addr_t buddy_index(addr_t x, int order)
{
    return ((x) / (Math::pow_2(order)));
}

constexpr size_t stack_overhead(size_t total_memory, size_t block_size)
{
    return total_memory / block_size /
           (Memory::Virtual::PAGE_SIZE / sizeof(addr_t));
}

Buddy::Buddy(size_t s, size_t min, size_t max)
{
    // TODO: Perform sanity checks
    this->size          = s;
    this->min_order     = min;
    this->max_order     = max;
    this->orders        = new BuddyOrder[max + 1];
    size_t stack_offset = 0;
    for (size_t i = min; i <= max; i++) {
        this->orders[i].free.set_base(
            reinterpret_cast<addr_t*>(STACK_START + stack_offset));
        stack_offset += Memory::Virtual::align_up(
            stack_overhead(this->size, Math::pow_2(i)));
        this->orders[i].bitset =
            new uint8_t[bitset_size_calc(this->size / Math::pow_2(i))];
        String::memset(this->orders[i].bitset, bitset_full,
                       bitset_size_calc(this->size / Math::pow_2(i)));
    }
}

Buddy::~Buddy()
{
}

addr_t Buddy::alloc(size_t size)
{
    size_t order = Math::log_2(size);
    if (order < this->min_order)
        order = this->min_order;
    if (order > this->max_order)
        return 0;
    size_t original_order = order;
    if (this->orders[order].free.empty()) {
        bool found = false;
        while (order++ <= this->max_order) {
            if (!this->orders[order].free.empty()) {
                found = true;
                break;
            }
        }
        if (!found) {
            return 0;
        }
        addr_t addr = this->orders[order].free.pop();
        for (; order > original_order; order--) {
            bitset_set(this->orders[order].bitset, buddy_index(addr, order));
            bitset_set(this->orders[order - 1].bitset,
                       buddy_index(addr, order - 1));
            this->orders[order - 1].free.push(buddy_address(addr, order - 1));
        }
        return addr;
    } else {
        addr_t addr = this->orders[order].free.pop();
        bitset_set(this->orders[order].bitset, buddy_index(addr, order));
        return addr;
    }
}

void Buddy::free(addr_t addr, size_t size)
{
    uint32_t order = Math::log_2(size);
    for (; order <= this->max_order; order++) {
        bitset_unset(this->orders[order].bitset, buddy_index(addr, order));
        addr_t buddy_addr = buddy_address(addr, order);
        if (bitset_test(this->orders[order].bitset,
                        buddy_index(buddy_addr, order)) ||
            order == this->max_order) {
            this->orders[order].free.push(addr);
            break;
        }
    }
}