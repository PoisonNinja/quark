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
extern bool arch_map(addr_t v, addr_t p, int flags);
extern bool arch_protect(addr_t v, int flags);
extern bool arch_unmap(addr_t v);
extern bool arch_test(addr_t v);
extern status_t arch_fork();
extern addr_t arch_get_address_space_root();
extern void arch_set_address_space_root(addr_t root);

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

bool protect(addr_t v, int flags)
{
    v = Memory::Virtual::align_down(v);
    return Memory::Virtual::arch_protect(v, flags);
}

bool protect(addr_t v, size_t size, int flags)
{
    v = Memory::Virtual::align_down(v);
    size = Memory::Virtual::align_up(size);
    for (addr_t i = 0; i < size; i += PAGE_SIZE) {
        if (!Memory::Virtual::arch_protect(v + i, flags)) {
            return false;
        }
    }
    return true;
}

bool unmap(addr_t v)
{
    v = Memory::Virtual::align_down(v);
    return Memory::Virtual::arch_unmap(v);
}

bool unmap(addr_t v, size_t size)
{
    v = Memory::Virtual::align_down(v);
    size = Memory::Virtual::align_up(size);
    for (addr_t i = 0; i < size; i += PAGE_SIZE) {
        if (!Memory::Virtual::arch_unmap(v + i)) {
            return false;
        }
    }
    return true;
}

bool test(addr_t v)
{
    v = Memory::Virtual::align_down(v);
    return Memory::Virtual::arch_test(v);
}

addr_t fork()
{
    return arch_fork();
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
