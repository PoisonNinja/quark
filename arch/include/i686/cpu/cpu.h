#pragma once

#include <arch/common/cpu/cpu.h>
#include <types.h>

namespace CPU
{
namespace X86
{
static inline void wrmsr(uint32_t msr_id, uint64_t msr_value)
{
    asm volatile("wrmsr" : : "c"(msr_id), "A"(msr_value));
}

static inline uint64_t rdmsr(uint32_t msr_id)
{
    uint64_t msr_value;
    asm volatile("rdmsr" : "=A"(msr_value) : "c"(msr_id));
    return msr_value;
}

void init();
}  // namespace X86
}  // namespace CPU
