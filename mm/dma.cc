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
    region.real_size     = size;
    if (!Memory::Virtual::map_range(region.virtual_base, region.physical_base,
                                    region.size, PAGE_WRITABLE)) {
        Memory::Valloc::free(region.virtual_base);
        Memory::Physical::free(region.physical_base, region.size);
        return false;
    }
    return true;
}

SGList* build_sglist(size_t max_elements, size_t max_element_size,
                     size_t total_size)
{
    SGList* sglist   = new SGList();
    size_t allocated = 0;
    while (allocated < total_size && sglist->num_regions <= max_elements) {
        // We always try to allocate the maximum size
        size_t alloc_size =
            (max_element_size < total_size) ? max_element_size : total_size;
        auto alloc            = Memory::Physical::try_allocate(alloc_size);
        Region* region        = new Region();
        region->physical_base = alloc.first;
        region->real_size     = alloc.second;
        allocated += alloc.second;

        if (allocated < total_size) {
            region->size = alloc.second;
        } else {
            region->size = total_size - (allocated - alloc.second);
        }

        region->virtual_base = Memory::Valloc::allocate(alloc.second);
        Memory::Virtual::map_range(region->virtual_base, region->physical_base,
                                   region->size, PAGE_WRITABLE);
        sglist->list.push_back(*region);
        sglist->num_regions++;
        sglist->total_size += region->size;
    }
    return sglist;
}
} // namespace DMA
} // namespace Memory