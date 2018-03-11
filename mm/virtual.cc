#include <arch/mm/mm.h>
#include <kernel.h>
#include <mm/mm.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
extern bool arch_map(addr_t v, addr_t p, int flags);

bool map(addr_t v, addr_t p, int flags)
{
    return Memory::Virtual::arch_map(v, p, flags);
}

bool map(addr_t v, addr_t p, size_t size, int flags)
{
    for (addr_t i = 0; i < Memory::Virtual::align_up(size); i += PAGE_SIZE) {
        if (!map(v + i, p + i, flags)) {
            return false;
        }
    }
    return true;
}

extern status_t arch_clone();

addr_t clone()
{
    return arch_clone();
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
