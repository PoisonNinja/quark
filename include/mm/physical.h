#pragma once

#include <boot/info.h>
#include <lib/utility.h>
#include <types.h>

namespace memory
{
namespace physical
{
addr_t allocate();
addr_t allocate(size_t size);
void free(addr_t address);
void free(addr_t address, size_t size);

size_t available();

/*
 * Tries allocating chunks of memory starting at max_size and slowly decreasing
 * size until we finally get some memory
 *
 * Returns a pair, with first element the address and second element size of the
 * block
 */
libcxx::pair<addr_t, size_t> try_allocate(size_t max_size);

void init(boot::info& info);
void finalize();
} // namespace physical
} // namespace memory
