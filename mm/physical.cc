#include <kernel.h>
#include <lib/utility.h>
#include <mm/buddy.h>
#include <mm/mm.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace memory
{
namespace physical
{
namespace
{
buddy* allocator = nullptr;
bool online      = false;

constexpr size_t minimum_order = 12; // 4 KiB
constexpr size_t maximum_order = 28; // 256 MiB
};                                   // namespace

addr_t early_allocate();

void init(boot::info& info)
{
    allocator = new buddy(info.highest, minimum_order, maximum_order);
}

addr_t allocate()
{
    if (!online) {
        return early_allocate();
    }
    addr_t result = allocator->alloc(memory::virt::PAGE_SIZE);
    // TODO: Perform sanity checks
    return result;
}

addr_t allocate(size_t size)
{
    /*
     * It doesn't make sense to try to satisfy this request early since the
     * early allocator only supports handing out pages
     */
    if (!online) {
        return 0;
    }
    addr_t result = allocator->alloc(size);
    return result;
}

void free(addr_t address)
{
    // TODO: Round address
    allocator->free(address, memory::virt::PAGE_SIZE);
}

void free(addr_t address, size_t size)
{
    allocator->free(address, size);
}

size_t available()
{
    return allocator->total();
}

/*
 * Unfortunately there is no way to implement this without somewhat relying on
 * the properties of the buddy allocator, so this probably won't even work if
 * we ever change physical memory algorithms.
 */
libcxx::pair<addr_t, size_t> try_allocate(size_t max_size)
{
    if (max_size <= memory::virt::PAGE_SIZE)
        max_size = memory::virt::PAGE_SIZE;
    size_t rounded_size = libcxx::pow2(libcxx::log2(max_size));
    while (rounded_size >= memory::virt::PAGE_SIZE) {
        if (allocator->available(rounded_size)) {
            return libcxx::make_pair(allocator->alloc(rounded_size),
                                     rounded_size);
        }
        rounded_size /= 2;
    }
    kernel::panic("Out of memory!\n");
}

void finalize()
{
    online = true;
}
} // namespace physical
} // namespace memory
