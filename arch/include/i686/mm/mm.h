#pragma once

#include <types.h>

namespace Memory
{
namespace X86
{
constexpr addr_t recursive_entry = 1023;
constexpr addr_t copy_entry = 1020;

constexpr addr_t pd_index(addr_t x)
{
    return ((x >> 22) & 0x3FF);
}
constexpr addr_t pt_index(addr_t x)
{
    return ((x >> 12) & 0x3FF);
}

static inline addr_t read_cr3(void)
{
    addr_t value;
    __asm__("mov %%cr3, %%eax" : "=a"(value));
    return value;
}

static inline void write_cr3(addr_t value)
{
    __asm__("mov %%eax, %%cr3" : : "a"(value));
}

static inline void invlpg(addr_t addr)
{
    __asm__ __volatile__("invlpg (%0)" ::"r"(addr) : "memory");
}

static inline void* decode_fractal(uint32_t pd, uint32_t pt)
{
    uint32_t address = (pd << 22);
    address |= (pt << 12);
    return (void*)address;
}

}  // namespace X64
}  // namespace Memory