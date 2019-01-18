#pragma once

#include <types.h>

namespace boot
{
struct info;
}

namespace memory
{
namespace X86
{
// Slot #s
#ifdef X86_64
constexpr addr_t recursive_entry = 510;
constexpr addr_t copy_entry      = 508;
#else
constexpr addr_t recursive_entry = 1023;
constexpr addr_t copy_entry      = 1020;
#endif

/*
 * x86_64 and x86 differ slightly in the number of bits and offset for each
 * index
 */
#ifdef X86_64
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
#else
constexpr addr_t pd_index(addr_t x)
{
    return ((x >> 22) & 0x3FF);
}
constexpr addr_t pt_index(addr_t x)
{
    return ((x >> 12) & 0x3FF);
}
#endif

static inline addr_t read_cr3(void)
{
    addr_t value;
#ifdef X86_64
    __asm__("mov %%cr3, %%rax" : "=a"(value));
#else
    __asm__("mov %%cr3, %%eax" : "=a"(value));
#endif
    return value;
}

static inline void write_cr3(addr_t value)
{
#ifdef X86_64
    __asm__("mov %%rax, %%cr3" : : "a"(value));
#else
    __asm__("mov %%eax, %%cr3" : : "a"(value));
#endif
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
} // namespace X86
} // namespace memory
