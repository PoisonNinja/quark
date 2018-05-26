#include <arch/mm/mm.h>
#include <kernel.h>
#include <mm/mm.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
// Architecture-dependent interface
extern addr_t arch_get_address_space_root();
extern void arch_set_address_space_root(addr_t root);

bool map(addr_t v, int flags)
{
    v = Memory::Virtual::align_down(v);
    return Memory::Virtual::map(v, Memory::Physical::allocate(), flags);
}

bool map_range(addr_t v, addr_t p, size_t size, int flags)
{
    v = Memory::Virtual::align_down(v);
    size = Memory::Virtual::align_up(size);
    for (addr_t i = 0; i < size; i += PAGE_SIZE) {
        if (!map(v + i, p + i, flags)) {
            return false;
        }
    }
    return true;
}

bool map_range(addr_t v, size_t size, int flags)
{
    v = Memory::Virtual::align_down(v);
    size = Memory::Virtual::align_up(size);
    for (addr_t i = 0; i < size; i += PAGE_SIZE) {
        if (!map(v + i, flags)) {
            return false;
        }
    }
    return true;
}

bool protect_range(addr_t v, size_t size, int flags)
{
    v = Memory::Virtual::align_down(v);
    size = Memory::Virtual::align_up(size);
    for (addr_t i = 0; i < size; i += PAGE_SIZE) {
        if (!Memory::Virtual::protect(v + i, flags)) {
            return false;
        }
    }
    return true;
}

bool unmap_range(addr_t v, size_t size)
{
    v = Memory::Virtual::align_down(v);
    size = Memory::Virtual::align_up(size);
    for (addr_t i = 0; i < size; i += PAGE_SIZE) {
        if (!Memory::Virtual::unmap(v + i)) {
            return false;
        }
    }
    return true;
}

addr_t get_address_space_root()
{
    return arch_get_address_space_root();
}

void set_address_space_root(addr_t root)
{
    return arch_set_address_space_root(root);
}
}  // namespace Virtual
}  // namespace Memory
