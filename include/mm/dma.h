#pragma once

#include <lib/vector.h>
#include <types.h>

namespace Memory
{
namespace DMA
{
struct Region {
    addr_t virtual_base;
    addr_t physical_base;
    addr_t size;

    // Please don't touch this
    size_t real_size;
};

struct SGList {
    SGList(size_t max_elements, size_t max_element_size, size_t total_size);
    ~SGList();
    size_t num_regions;
    size_t total_size;
    libcxx::vector<Region> list;
};

bool allocate(size_t size, Region& region);

/*
 * Builds a scatter gather list with either max_elements regions or regions
 * adding up to total size, whichever comes first
 *
 * Callers must be responsible for memory allocated, including clearing the
 * region memory
 */
SGList* make_sglist(size_t max_elements, size_t max_element_size,
                    size_t total_size);
} // namespace DMA
} // namespace Memory