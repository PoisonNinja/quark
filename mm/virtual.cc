#include <arch/mm/mm.h>
#include <kernel.h>
#include <mm/mm.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
extern bool arch_map(addr_t v, addr_t p, int flags);

bool map(addr_t v, addr_t p, int flags)
{
    v = Memory::Virtual::align_down(v);
    return Memory::Virtual::arch_map(v, p, flags);
}

bool map(addr_t v, addr_t p, size_t size, int flags)
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

bool map(addr_t v, int flags)
{
    v = Memory::Virtual::align_down(v);
    return Memory::Virtual::arch_map(v, Memory::Physical::allocate(), flags);
}

extern status_t arch_update(addr_t v, int flags);

status_t update(addr_t v, int flags)
{
    v = Memory::Virtual::align_down(v);
    return arch_update(v, flags);
}

extern status_t arch_fork();

addr_t fork()
{
    return arch_fork();
}

extern addr_t arch_get_address_space_root();

addr_t get_address_space_root()
{
    return arch_get_address_space_root();
}

extern void arch_set_address_space_root(addr_t root);

void set_address_space_root(addr_t root)
{
    return arch_set_address_space_root(root);
}
}  // namespace Virtual
}  // namespace Memory
