#include <arch/mm/mm.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/mm.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Physical
{
static size_t stack_used = 0;
static size_t stack_size = Memory::Virtual::PAGE_SIZE / sizeof(addr_t);

static void expand_stack()
{
    addr_t phys = Memory::Physical::get();
    addr_t virt = reinterpret_cast<addr_t>(STACK + stack_size);
    Memory::Virtual::map(virt, phys, PAGE_PRESENT | PAGE_WRITABLE);
    stack_size += Memory::Virtual::PAGE_SIZE / sizeof(addr_t);
}

addr_t get()
{
    addr_t result = STACK[--stack_used];
    // TODO: Perform sanity checks
    return result;
}

void put(addr_t address)
{
    // TODO: Round address
    if (stack_used == stack_size) {
        expand_stack();
    }
    STACK[stack_used++] = address;
}

void put_range(addr_t base, size_t size)
{
    for (addr_t i = base; i < base + size; i += Memory::Virtual::PAGE_SIZE) {
        Memory::Physical::put(i);
    }
}
}  // namespace Physical
}  // namespace Memory
