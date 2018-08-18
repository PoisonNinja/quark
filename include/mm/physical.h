#pragma once

#include <boot/info.h>
#include <types.h>

namespace Memory
{
namespace Physical
{
addr_t allocate();
addr_t allocate(size_t size);
void free(addr_t address);
void free(addr_t address, size_t size);

void init(Boot::info& info);
void finalize();
} // namespace Physical
} // namespace Memory
