#pragma once

#include <boot/info.h>
#include <types.h>

namespace Memory
{
namespace Physical
{
addr_t allocate();
void free(addr_t address);

void put_range(addr_t base, size_t size);

void init(Boot::info& info);
void finalize();
}  // namespace Physical
}  // namespace Memory
