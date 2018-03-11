#pragma once

#include <arch/mm/mm.h>
#include <types.h>

namespace Memory
{
namespace Virtual
{
#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4
#define PAGE_GLOBAL 0x8
#define PAGE_NX 0x10
#define PAGE_HUGE 0x20
#define PAGE_COW 0x40

bool map(addr_t v, addr_t p, int flags);
bool map(addr_t v, addr_t p, size_t size, int flags);
addr_t clone();

addr_t get_address_space_root();
void set_address_space_root(addr_t root);
}  // namespace Virtual
}  // namespace Memory
