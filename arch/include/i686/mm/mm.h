#pragma once

#include <types.h>

namespace Memory
{
namespace X86
{
constexpr addr_t recursive_entry = 510;
constexpr addr_t copy_entry = 508;

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

static inline void invlpg(addr_t addr)
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
