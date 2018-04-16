#pragma once

#include <types.h>

namespace CPU
{
namespace X64
{
constexpr uint64_t msr_efer = 0xC0000080;

constexpr uint64_t msr_star = 0xC0000081;
constexpr uint64_t msr_lstar = 0xC0000082;
constexpr uint64_t msr_cstar = 0xC0000083;
constexpr uint64_t msr_fmask = 0xC0000084;

constexpr uint64_t msr_kernel_gs_base = 0xC0000102;

static inline void wrmsr(uint64_t msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static inline uint64_t rdmsr(uint64_t msr)
{
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

void init();
}
}
