#pragma once

#include <arch/common/mm/mm.h>
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

static inline void* decode_fractal(uint32_t pd, uint32_t pt)
{
    uint32_t address = (pd << 22);
    address |= (pt << 12);
    return (void*)address;
}

}  // namespace X86
}  // namespace Memory
