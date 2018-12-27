#pragma once

#include <types.h>

namespace memory
{
namespace vmalloc
{
addr_t allocate(size_t size);
void free(addr_t address);

void init();
} // namespace vmalloc
} // namespace memory