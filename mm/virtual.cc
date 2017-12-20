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
    Log::printk(Log::DEBUG, "map: %p -> %p, %X\n", v, p, flags);
    return Memory::Virtual::arch_map(v, p, flags);
}
}  // namespace Virtual
}  // namespace Memory
