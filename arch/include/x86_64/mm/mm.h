#pragma once

#include <types.h>

namespace Memory
{
namespace Virtual
{
const size_t PAGE_SIZE = 4096;

inline addr_t align_up(addr_t address)
{
    return ((address + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
}

inline addr_t align_down(addr_t address)
{
    return ((address / PAGE_SIZE) * PAGE_SIZE);
}

struct page {
    uint32_t present : 1;
    uint32_t writable : 1;
    uint32_t user : 1;
    uint32_t writethrough_cache : 1;
    uint32_t disable_cache : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t huge_page : 1;
    uint32_t global : 1;
    uint32_t cow : 1;
    uint32_t available_1 : 2;
    uint64_t address : 40;
    uint32_t available_2 : 11;
    uint32_t nx : 1;
};

struct page_table {
    struct page pages[512];
};
}  // namespace Virtual

namespace Physical
{
addr_t* const STACK = reinterpret_cast<addr_t*>(0xFFFFFE8000000000);
}

namespace X64
{
#define PML4_INDEX(x) ((x >> 39) & 0x1FF)
#define PDPT_INDEX(x) ((x >> 30) & 0x1FF)
#define PD_INDEX(x) ((x >> 21) & 0x1FF)
#define PT_INDEX(x) ((x >> 12) & 0x1FF)

#define RECURSIVE_ENTRY (510UL)
#define COPY_ENTRY (508UL)

static inline uint64_t read_cr3(void)
{
    uint64_t value;
    __asm__("mov %%cr3, %%rax" : "=a"(value));
    return value;
}

static inline void write_cr3(uint64_t value)
{
    __asm__("mov %%rax, %%cr3" : : "a"(value));
}

static inline void invlpg(void* addr)
{
    __asm__ __volatile__("invlpg (%0)" ::"r"(addr) : "memory");
}

/*
 * Convert a table entry into the recursive mapping address. Taken from
 * some forum post on osdev.org
 */
static inline void* decode_fractal(uint64_t pml4, uint64_t pdp, uint64_t pd,
                                   uint64_t pt)
{
    uint64_t address = (pml4 << 39);

    if ((address & (1ll << 47)) > 0) {
        // We need to sign extend
        address |= 0xFFFF000000000000UL;
    }

    address |= pdp << 30;
    address |= pd << 21;
    address |= pt << 12;
    return (void*)address;
}
}  // namespace X64
}  // namespace Memory
