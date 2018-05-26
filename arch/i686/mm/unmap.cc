#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <mm/physical.h>

namespace Memory
{
namespace Virtual
{
static inline bool __table_is_empty(struct page_table* table)
{
    for (int i = 0; i < 1024; i++) {
        if (table->pages[i].present)
            return false;
    }
    return true;
}

bool unmap(addr_t v)
{
    struct page_table* pd = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::recursive_entry);
    struct page_table* pt = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::pd_index(v));
    if (!pd->pages[Memory::X86::pd_index(v)].present ||
        !pt->pages[Memory::X86::pt_index(v)].present)
        return false;
    if (!pt->pages[Memory::X86::pt_index(v)].hardware) {
        Memory::Physical::free(pt->pages[Memory::X86::pt_index(v)].address *
                               0x1000);
    }
    pt->pages[Memory::X86::pt_index(v)].present = 0;
    if (__table_is_empty(pt)) {
        Memory::Physical::free(pd->pages[Memory::X86::pd_index(v)].address *
                               0x1000);
        pd->pages[Memory::X86::pd_index(v)].present = 0;
    }
    return true;
}
}
}
