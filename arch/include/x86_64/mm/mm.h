#pragma once

#include <types.h>

namespace boot
{
struct info;
}

namespace memory
{
namespace x86
{
// Slot #s
constexpr addr_t recursive_entry = 510;
constexpr addr_t copy_entry      = 508;

constexpr addr_t pml4_index(addr_t x)
{
    return ((x >> 39) & 0x1FF);
}
constexpr addr_t pdpt_index(addr_t x)
{
    return ((x >> 30) & 0x1FF);
}
constexpr addr_t pd_index(addr_t x)
{
    return ((x >> 21) & 0x1FF);
}
constexpr addr_t pt_index(addr_t x)
{
    return ((x >> 12) & 0x1FF);
}

static inline addr_t read_cr3(void)
{
    addr_t value;
    __asm__("mov %%cr3, %%rax" : "=a"(value));
    return value;
}

static inline void write_cr3(addr_t value)
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
constexpr void* decode_fractal(uint64_t pml4, uint64_t pdp, uint64_t pd,
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
    return reinterpret_cast<void*>(address);
}

#ifndef X86_64
constexpr void* decode_fractal(uint32_t pd, uint32_t pt)
{
    uint32_t address = (pd << 22);
    address |= (pt << 12);
    return reinterpret_cast<void*>(address);
}
#endif

bool is_valid_physical_memory(addr_t m, struct boot::info& info);
} // namespace x86
} // namespace memory
