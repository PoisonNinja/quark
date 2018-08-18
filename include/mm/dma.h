#pragma once

#include <types.h>

namespace Memory
{
namespace DMA
{
struct Region {
    addr_t virtual_base;
    addr_t physical_base;
    size_t size;
};

bool allocate(size_t size, Region& region);
} // namespace DMA
} // namespace Memory