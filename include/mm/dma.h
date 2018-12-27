#pragma once

#include <lib/vector.h>
#include <types.h>

namespace Memory
{
namespace dma
{
struct region {
    addr_t virtual_base;
    addr_t physical_base;
    addr_t size;

    // Please don't touch this
    size_t real_size;
};

struct sglist {
    sglist(size_t max_elements, size_t max_element_size, size_t total_size);
    ~sglist();
    size_t num_regions;
    size_t total_size;
    libcxx::vector<region> list;
};

bool allocate(size_t size, region& region);

/*
 * Builds a scatter gather list with either max_elements regions or regions
 * adding up to total size, whichever comes first
 *
 * Callers must be responsible for memory allocated, including clearing the
 * region memory
 */
sglist* make_sglist(size_t max_elements, size_t max_element_size,
                    size_t total_size);
} // namespace dma
} // namespace Memory