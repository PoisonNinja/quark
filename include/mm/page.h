#pragma once

#include <boot/info.h>

namespace memory
{
struct page {
    struct page *next, *prev;
};

namespace pagedb
{
void init(boot::info& info);
struct page* get(addr_t address);
addr_t addr(struct page* page);
} // namespace pagedb
} // namespace memory
