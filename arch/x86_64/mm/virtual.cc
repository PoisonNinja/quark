#include <arch/mm/mm.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
addr_t arch_get_address_space_root()
{
    return Memory::X64::read_cr3();
}

void arch_set_address_space_root(addr_t root)
{
    return Memory::X64::write_cr3(root);
}
}
}
