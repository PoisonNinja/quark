#include <mm/dma.h>
#include <mm/physical.h>
#include <mm/valloc.h>
#include <mm/virtual.h>

namespace Memory
{
namespace DMA
{
bool allocate(size_t size, Region& region)
{
    region.virtual_base  = Memory::Valloc::allocate(size);
    region.physical_base = Memory::Physical::allocate(size);
    region.size          = size;
    if (!Memory::Virtual::map_range(region.virtual_base, region.physical_base,
                                    region.size, PAGE_WRITABLE)) {
        Memory::Valloc::free(region.virtual_base);
        Memory::Physical::free(region.physical_base, region.size);
        return false;
    }
    return true;
}
} // namespace DMA
} // namespace Memory