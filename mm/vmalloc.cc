#include <arch/mm/layout.h>
#include <kernel.h>
#include <mm/virtual.h>
#include <mm/vma.h>
#include <mm/vmalloc.h>

namespace Memory
{
namespace vmalloc
{
namespace
{
Memory::vma vmalloc_region(VMALLOC_START, VMALLOC_END);
}

addr_t allocate(size_t size)
{
    size       = Memory::Virtual::align_up(size);
    addr_t ret = 0;
    if (!vmalloc_region.locate_range(ret, 0, size)) {
        Log::printk(Log::LogLevel::WARNING, "vmalloc: Out of memory somehow\n");
    }
    return ret;
}

void free(addr_t address)
{
    Log::printk(Log::LogLevel::WARNING,
                "vmalloc memory free was requested, but this "
                "function is not implemented!\n");
}
} // namespace vmalloc
} // namespace Memory