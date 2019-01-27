#include <arch/mm/layout.h>
#include <kernel.h>
#include <mm/dma.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <mm/vma.h>
#include <mm/vmalloc.h>

namespace memory
{
namespace dma
{
namespace
{
memory::vma dma_region(DMA_START, DMA_END);
}

libcxx::pair<bool, region> allocate(size_t size)
{
    region region;
    auto [virt_found, virt_address] = dma_region.allocate(0, size);
    if (!virt_found) {
        log::printk(log::log_level::WARNING,
                    "dma: Unable to find free space\n");
        return libcxx::make_pair(false, region);
    }
    region.virtual_base  = virt_address;
    region.physical_base = memory::physical::allocate(size);
    region.size          = size;
    region.real_size     = size;
    if (!memory::virt::map_range(region.virtual_base, region.physical_base,
                                 region.size, PAGE_WRITABLE)) {
        memory::vmalloc::free(region.virtual_base);
        memory::physical::free(region.physical_base, region.size);
        return libcxx::make_pair(false, region);
    }
    return libcxx::make_pair(true, region);
}

sglist::sglist(size_t max_elements, size_t max_element_size, size_t total)
    : num_regions(0)
    , total_size(0)
{
    size_t allocated = 0;
    while (allocated < total && num_regions <= max_elements) {
        // We always try to allocate the maximum size
        size_t alloc_size =
            (max_element_size < total) ? max_element_size : total;
        auto [physical_base, real_size] =
            memory::physical::try_allocate(alloc_size);
        region region;
        region.physical_base = physical_base;
        region.real_size     = real_size;
        allocated += real_size;

        if (allocated < total) {
            region.size = real_size;
        } else {
            region.size = total - (allocated - real_size);
        }

        region.virtual_base = dma_region.allocate(0, real_size).second;
        memory::virt::map_range(region.virtual_base, region.physical_base,
                                region.real_size, PAGE_WRITABLE);
        list.push_back(region);
        num_regions++;
        total_size += region.size;
    }
}

sglist::~sglist()
{
    for (auto region : this->list) {
        memory::virt::unmap_range(region.virtual_base, region.size);
        dma_region.free(region.virtual_base, region.real_size);
        memory::physical::free(region.physical_base, region.real_size);
    }
}

libcxx::unique_ptr<sglist>
make_sglist(size_t max_elements, size_t max_element_size, size_t total_size)
{
    // TODO: Use unique_ptr
    auto sg = libcxx::unique_ptr<sglist>(
        new sglist(max_elements, max_element_size, total_size));
    return sg;
}
} // namespace dma
} // namespace memory
