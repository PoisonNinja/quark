#include <kernel.h>
#include <mm/buddy.h>
#include <mm/mm.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Physical
{
namespace
{
Buddy* buddy = nullptr;
bool online  = false;

constexpr size_t minimum_order = 12; // 4 KiB
constexpr size_t maximum_order = 28; // 256 MiB
};                                   // namespace

addr_t early_allocate();

void init(Boot::info& info)
{
    buddy = new Buddy(info.highest, minimum_order, maximum_order);
}

addr_t allocate()
{
    if (!online) {
        return early_allocate();
    }
    addr_t result = buddy->alloc(Memory::Virtual::PAGE_SIZE);
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
    addr_t result = buddy->alloc(size);
    return result;
}

void free(addr_t address)
{
    // TODO: Round address
    buddy->free(address, Memory::Virtual::PAGE_SIZE);
}

void free(addr_t address, size_t size)
{
    buddy->free(address, size);
}

void finalize()
{
    online = true;
}
} // namespace Physical
} // namespace Memory
