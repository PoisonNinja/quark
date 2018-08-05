#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
addr_t arch_get_address_space_root()
{
    return Memory::X86::read_cr3();
}

void arch_set_address_space_root(addr_t root)
{
    return Memory::X86::write_cr3(root);
}
}  // namespace Virtual
}  // namespace Memory
