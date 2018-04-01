#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <mm/physical.h>

namespace Memory
{
namespace Virtual
{
static inline bool __table_is_empty(struct page_table* table)
{
    for (int i = 0; i < 512; i++) {
        if (table->pages[i].present)
            return false;
    }
    return true;
}

bool arch_unmap(addr_t v)
{
    struct page_table* pml4 = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    struct page_table* pdpt = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(v));
    struct page_table* pd = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(v), PDPT_INDEX(v));
    struct page_table* pt = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, PML4_INDEX(v), PDPT_INDEX(v), PD_INDEX(v));
    if (!pml4->pages[PML4_INDEX(v)].present ||
        !pdpt->pages[PDPT_INDEX(v)].present ||
        !pd->pages[PD_INDEX(v)].present || !pt->pages[PT_INDEX(v)].present)
        return false;
    if (!pt->pages[PT_INDEX(v)].hardware) {
        Memory::Physical::free(pt->pages[PT_INDEX(v)].address * 0x1000);
    }
    pt->pages[PT_INDEX(v)].present = 0;
    if (__table_is_empty(pt)) {
        Memory::Physical::free(pd->pages[PD_INDEX(v)].address * 0x1000);
        pd->pages[PD_INDEX(v)].present = 0;
        if (__table_is_empty(pd)) {
            Memory::Physical::free(pdpt->pages[PDPT_INDEX(v)].address * 0x1000);
            pdpt->pages[PDPT_INDEX(v)].present = 0;
            if (__table_is_empty(pdpt)) {
                Memory::Physical::free(pml4->pages[PML4_INDEX(v)].address *
                                       0x1000);
                pml4->pages[PML4_INDEX(v)].present = 0;
            }
        }
    }
    return true;
}
}
}
