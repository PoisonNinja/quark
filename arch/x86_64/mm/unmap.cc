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

bool unmap(addr_t v)
{
    struct page_table* pml4 = (struct page_table*)Memory::X64::decode_fractal(
        Memory::X64::recursive_entry, Memory::X64::recursive_entry,
        Memory::X64::recursive_entry, Memory::X64::recursive_entry);
    struct page_table* pdpt = (struct page_table*)Memory::X64::decode_fractal(
        Memory::X64::recursive_entry, Memory::X64::recursive_entry,
        Memory::X64::recursive_entry, Memory::X64::pml4_index(v));
    struct page_table* pd = (struct page_table*)Memory::X64::decode_fractal(
        Memory::X64::recursive_entry, Memory::X64::recursive_entry,
        Memory::X64::pml4_index(v), Memory::X64::pdpt_index(v));
    struct page_table* pt = (struct page_table*)Memory::X64::decode_fractal(
        Memory::X64::recursive_entry, Memory::X64::pml4_index(v),
        Memory::X64::pdpt_index(v), Memory::X64::pd_index(v));
    if (!pml4->pages[Memory::X64::pml4_index(v)].present ||
        !pdpt->pages[Memory::X64::pdpt_index(v)].present ||
        !pd->pages[Memory::X64::pd_index(v)].present ||
        !pt->pages[Memory::X64::pt_index(v)].present)
        return false;
    if (!pt->pages[Memory::X64::pt_index(v)].hardware) {
        Memory::Physical::free(pt->pages[Memory::X64::pt_index(v)].address *
                               0x1000);
    }
    pt->pages[Memory::X64::pt_index(v)].present = 0;
    if (__table_is_empty(pt)) {
        Memory::Physical::free(pd->pages[Memory::X64::pd_index(v)].address *
                               0x1000);
        pd->pages[Memory::X64::pd_index(v)].present = 0;
        if (__table_is_empty(pd)) {
            Memory::Physical::free(
                pdpt->pages[Memory::X64::pdpt_index(v)].address * 0x1000);
            pdpt->pages[Memory::X64::pdpt_index(v)].present = 0;
            if (__table_is_empty(pdpt)) {
                Memory::Physical::free(
                    pml4->pages[Memory::X64::pml4_index(v)].address * 0x1000);
                pml4->pages[Memory::X64::pml4_index(v)].present = 0;
            }
        }
    }
    return true;
}
}
}
