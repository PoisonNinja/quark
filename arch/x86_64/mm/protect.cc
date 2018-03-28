#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <mm/virtual.h>
#include <kernel.h>

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

bool arch_protect(addr_t v, int flags)
{
    struct page_table* pml4 = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    struct page_table* pdpt = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(v));
    struct page_table* pd = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(v), PDPT_INDEX(v));
    struct page_table* pt = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, PML4_INDEX(v), PDPT_INDEX(v), PD_INDEX(v));
    if (!pml4->pages[PML4_INDEX(v)].present) {
        return false;
    }
    __set_flags(&pml4->pages[PML4_INDEX(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    if (!pdpt->pages[PDPT_INDEX(v)].present) {
        return false;
    }
    __set_flags(&pdpt->pages[PDPT_INDEX(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    if (!pd->pages[PD_INDEX(v)].present) {
        return false;
    }
    __set_flags(&pd->pages[PD_INDEX(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    if (!pt->pages[PT_INDEX(v)].present) {
        return false;
    }
    __set_flags(&pt->pages[PT_INDEX(v)], flags);
    return SUCCESS;
}
}  // namespace Virtual
}  // namespace Memory
