#include <arch/mm/virtual.h>
#include <arch/mm/mm.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
static inline void __set_flags(struct page* page, uint8_t flags)
{
    page->present = 1;
    page->writable = (flags & PAGE_WRITABLE) ? 1 : 0;
    page->user = (flags & PAGE_USER) ? 1 : 0;
    page->global = (flags & PAGE_GLOBAL) ? 1 : 0;
    page->nx = (flags & PAGE_NX) ? 1 : 0;
}

status_t arch_update(addr_t v, int flags)
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
        !pdpt->pages[PML4_INDEX(v)].present ||
        !pd->pages[PML4_INDEX(v)].present || !pt->pages[PML4_INDEX(v)].present)
        return FAILURE;
    __set_flags(&pml4->pages[PML4_INDEX(v)], flags);
    __set_flags(&pdpt->pages[PDPT_INDEX(v)], flags);
    __set_flags(&pd->pages[PD_INDEX(v)], flags);
    __set_flags(&pt->pages[PT_INDEX(v)], flags);
    return SUCCESS;
}
}  // namespace Virtual
}  // namespace Memory
