#include <arch/mm/layout.h>
#include <kernel.h>
#include <lib/bitset.h>
#include <lib/string.h>
#include <mm/buddy.h>
#include <mm/virtual.h>

#define POW_2(power) (1 << (power))
#define BUDDY_ADDRESS(x, order) ((x) ^ (1 << (order)))
#define BUDDY_INDEX(x, order) ((x) / (POW_2(order)))

constexpr size_t stack_overhead(size_t total_memory, size_t block_size)
{
    return total_memory / block_size /
           (Memory::Virtual::PAGE_SIZE / sizeof(addr_t));
}

static inline uint32_t log_2(uint64_t x)
{
    static const uint64_t t[6] = {0xFFFFFFFF00000000ull, 0x00000000FFFF0000ull,
                                  0x000000000000FF00ull, 0x00000000000000F0ull,
                                  0x000000000000000Cull, 0x0000000000000002ull};

    int y = (((x & (x - 1)) == 0) ? 0 : 1);
    int j = 32;
    int i;

    for (i = 0; i < 6; i++) {
        int k = (((x & t[i]) == 0) ? 0 : j);
        y += k;
        x >>= k;
        j >>= 1;
    }

    return y;
}

Buddy::Buddy(size_t s, size_t min, size_t max)
{
    // TODO: Perform sanity checks
    this->size = s;
    this->min_order = min;
    this->max_order = max;
    this->orders = new BuddyOrder[max + 1];
    size_t stack_offset = 0;
    for (size_t i = min; i <= max; i++) {
        this->orders[i].free.set_base(
            reinterpret_cast<addr_t*>(STACK_START + stack_offset));
        stack_offset +=
            Memory::Virtual::align_up(stack_overhead(this->size, POW_2(i)));
        this->orders[i].bitset =
            new uint8_t[BITSET_SIZE_CALC(this->size / POW_2(i))];
        String::memset(this->orders[i].bitset, BITSET_FULL,
                       BITSET_SIZE_CALC(this->size / POW_2(i)));
    }
}

Buddy::~Buddy()
{
}

addr_t Buddy::alloc(size_t size)
{
    size_t order = log_2(size);
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
            bitset_set(this->orders[order].bitset, BUDDY_INDEX(addr, order));
            bitset_set(this->orders[order - 1].bitset,
                       BUDDY_INDEX(addr, order - 1));
            this->orders[order - 1].free.push(BUDDY_ADDRESS(addr, order - 1));
        }
        return addr;
    } else {
        addr_t addr = this->orders[order].free.pop();
        bitset_set(this->orders[order].bitset, BUDDY_INDEX(addr, order));
        return addr;
    }
}

void Buddy::free(addr_t addr, size_t size)
{
    uint32_t order = log_2(size);
    for (; order <= this->max_order; order++) {
        bitset_unset(this->orders[order].bitset, BUDDY_INDEX(addr, order));
        addr_t buddy_addr = BUDDY_ADDRESS(addr, order);
        if (bitset_test(this->orders[order].bitset,
                        BUDDY_INDEX(buddy_addr, order)) ||
            order == this->max_order) {
            this->orders[order].free.push(addr);
            break;
        }
    }
}