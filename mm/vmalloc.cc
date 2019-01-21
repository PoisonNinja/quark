#include <arch/mm/layout.h>
#include <kernel.h>
#include <mm/virtual.h>
#include <mm/vma.h>
#include <mm/vmalloc.h>

namespace memory
{
namespace vmalloc
{
namespace
{
memory::vma vmalloc_region(VMALLOC_START, VMALLOC_END);
}

addr_t allocate(size_t size)
{
    size                  = memory::virt::align_up(size);
    auto [found, address] = vmalloc_region.allocate(0, size);
    if (!found) {
        log::printk(log::log_level::WARNING,
                    "vmalloc: Out of memory somehow\n");
    }
    return address;
}

void free(addr_t address)
{
    log::printk(log::log_level::WARNING,
                "vmalloc memory free was requested, but this "
                "function is not implemented!\n");
}
} // namespace vmalloc
} // namespace memory
