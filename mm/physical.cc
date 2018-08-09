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
bool online = false;
};  // namespace

addr_t early_allocate();

void init(Boot::info& info)
{
    buddy = new Buddy(info.highest, 12, 28);
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

void free(addr_t address)
{
    // TODO: Round address
    buddy->free(address, Memory::Virtual::PAGE_SIZE);
}

void finalize()
{
    online = true;
}
}  // namespace Physical
}  // namespace Memory
