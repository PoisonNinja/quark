#pragma once

#include <lib/list.h>
#include <types.h>

namespace Memory
{
namespace DMA
{
struct Region {
    addr_t virtual_base;
    addr_t physical_base;
    addr_t size;
    libcxx::node<Region> node;

    // Please don't touch this
    size_t real_size;
};

struct SGList {
    size_t num_regions;
    size_t total_size;
    libcxx::list<Region, &Region::node> list;
};

bool allocate(size_t size, Region& region);

/*
 * Builds a scatter gather list with either max_elements regions or regions
 * adding up to total size, whichever comes first
 *
 * Callers must be responsible for memory allocated, including clearing the
 * region memory
 */
SGList* build_sglist(size_t max_elements, size_t max_element_size,
                     size_t total_size);
} // namespace DMA
} // namespace Memory