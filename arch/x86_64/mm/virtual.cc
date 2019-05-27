#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <mm/virtual.h>

namespace memory
{
namespace virt
{
addr_t arch_get_address_space_root()
{
    return memory::x86_64::read_cr3();
}

void arch_set_address_space_root(addr_t root)
{
    return memory::x86_64::write_cr3(root);
}
} // namespace virt
} // namespace memory
