#pragma once

#include <types.h>

namespace Memory
{
namespace Physical
{
addr_t allocate();
void free(addr_t address);

void put_range(addr_t base, size_t size);
}  // namespace Physical
}  // namespace Memory
