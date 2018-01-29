#include <arch/mm/mm.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
status_t arch_get(addr_t v, struct page& page)
{
    struct page_table* pml4 =
        reinterpret_cast<struct page_table*>(Memory::X64::read_cr3());
    if (!pml4->pages[PML4_INDEX(v)].present) {
        return FAILURE;
    }
    struct page_table* pdpt = reinterpret_cast<struct page_table*>(
        pml4->pages[PML4_INDEX(v)].address * 0x1000);
    if (!pdpt->pages[PDPT_INDEX(v)].present) {
        return FAILURE;
    }
    struct page_table* pd = reinterpret_cast<struct page_table*>(
        pdpt->pages[PDPT_INDEX(v)].address * 0x1000);
    if (!pd->pages[PD_INDEX(v)].present) {
        return FAILURE;
    }
    struct page_table* pt = reinterpret_cast<struct page_table*>(
        pd->pages[PD_INDEX(v)].address * 0x1000);
    if (!pt->pages[PT_INDEX(v)].present) {
        return FAILURE;
    }
    page = pt->pages[PT_INDEX(v)];
    return SUCCESS;
}
}
}