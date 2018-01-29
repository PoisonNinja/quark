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

extern status_t arch_get(addr_t v, struct page& page);

status_t get(addr_t v, struct page& page)
{
    return Memory::Virtual::arch_get(v, page);
}
}  // namespace Virtual
}  // namespace Memory
