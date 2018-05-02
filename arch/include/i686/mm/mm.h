#pragma once

#include <types.h>

namespace Memory
{
namespace X86
{
constexpr addr_t recursive_entry = 510;
constexpr addr_t copy_entry = 508;

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

}  // namespace X64
}  // namespace Memory
