#include <arch/mm/layout.h>
#include <kernel.h>
#include <lib/utility.h>
#include <mm/mm.h>
#include <mm/physical.h>
#include <mm/stack.h>
#include <mm/virtual.h>

namespace memory
{
namespace physical
{
namespace
{
addr_t* base = reinterpret_cast<addr_t*>(STACK_START);
size_t used  = 0;
size_t size  = memory::virt::PAGE_SIZE / sizeof(addr_t);

addr_t last = 0;

bool online = false;

void expand()
{
    addr_t phys = memory::physical::allocate();
    addr_t virt = reinterpret_cast<addr_t>(base + size);
    memory::virt::map(virt, phys, PAGE_WRITABLE);
    size += memory::virt::PAGE_SIZE / sizeof(addr_t);
}
}; // namespace

addr_t early_allocate();

void init(boot::info& info)
{
}

addr_t allocate()
{
    if (!online) {
        return early_allocate();
    }
    if (base[used - 1] == last) {
        kernel::panic("Handed out same address twice, top at %p\n",
                      base + used);
    }
    last = base[used - 1];
    return base[--used];
}

void free(addr_t address)
{
    if (used == size) {
        expand();
    }
    if (address == last)
        last = 0;
    base[used++] = address;
}

void finalize()
{
    online = true;
}
} // namespace physical
} // namespace memory
