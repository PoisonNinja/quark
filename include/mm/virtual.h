#pragma once

#include <arch/mm/virtual.h>
#include <lib/math.h>
#include <types.h>

namespace Memory
{
namespace Virtual
{
#define PAGE_WRITABLE 0x1
#define PAGE_USER 0x2
#define PAGE_GLOBAL 0x4
#define PAGE_NX 0x8
#define PAGE_HUGE 0x10
#define PAGE_COW 0x20
#define PAGE_HARDWARE 0x40

// Userspace flags
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

#define MAP_SHARED 0x001
#define MAP_PRIVATE 0x002
#define MAP_FIXED 0x010
#define MAP_FILE 0x000
#define MAP_ANONYMOUS 0x020
#define MAP_ANON MAP_ANONYMOUS

#define MAP_FAILED (void *)-1

inline addr_t align_up(addr_t address)
{
    return libcxx::round_up(address, PAGE_SIZE);
}

inline addr_t align_down(addr_t address)
{
    return libcxx::round_down(address, PAGE_SIZE);
}

bool map(addr_t v, addr_t p, int flags);
bool map(addr_t v, int flags);
bool map_range(addr_t v, addr_t p, size_t size, int flags);
bool map_range(addr_t v, size_t size, int flags);

bool protect(addr_t v, int flags);
bool protect_range(addr_t v, size_t size, int flags);

bool unmap(addr_t v);
bool unmap_range(addr_t v, size_t size);

bool test(addr_t v);

addr_t fork();

// Translate userspace protection flags to kernel flags
inline int prot_to_flags(int prot)
{
    int flags = PAGE_NX;
    if (prot & PROT_READ) {
        flags |= PAGE_USER;
    }
    if (prot & PROT_WRITE) {
        flags |= PAGE_WRITABLE;
    }
    if (prot & PROT_EXEC) {
        flags &= ~PAGE_NX;
    }
    return flags;
}

addr_t get_address_space_root();
void set_address_space_root(addr_t root);
} // namespace Virtual
} // namespace Memory
