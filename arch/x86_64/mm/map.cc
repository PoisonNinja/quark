#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
static inline int __set_address(struct Memory::Virtual::page* page)
{
    if (!page->present) {
        page->address = Memory::Physical::allocate() / 0x1000;
        return 1;
    }
    return 0;
}

static inline void __set_flags(struct page* page, uint8_t flags)
{
    page->present = 1;
    page->writable = (flags & PAGE_WRITABLE) ? 1 : 0;
    page->user = (flags & PAGE_USER) ? 1 : 0;
    page->global = (flags & PAGE_GLOBAL) ? 1 : 0;
    page->nx = (flags & PAGE_NX) ? 1 : 0;
}

bool arch_map(addr_t v, addr_t p, int flags)
{
    struct page_table* pml4 = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    struct page_table* pdpt = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(v));
    struct page_table* pd = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(v), PDPT_INDEX(v));
    struct page_table* pt = (struct page_table*)Memory::X64::decode_fractal(
        RECURSIVE_ENTRY, PML4_INDEX(v), PDPT_INDEX(v), PD_INDEX(v));
    int r = 0;
    r = __set_address(&pml4->pages[PML4_INDEX(v)]);
    __set_flags(&pml4->pages[PML4_INDEX(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    /*
     * __set_address returns whether it allocated memory or not.
     * If it did, we need to memset it ourselves to 0.
     */
    if (r) {
        String::memset(pdpt, 0, sizeof(struct page_table));
    }
    r = __set_address(&pdpt->pages[PDPT_INDEX(v)]);
    __set_flags(&pdpt->pages[PDPT_INDEX(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    if (r) {
        String::memset(pd, 0, sizeof(struct page_table));
    }
    r = __set_address(&pd->pages[PD_INDEX(v)]);
    __set_flags(&pd->pages[PD_INDEX(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    if (r) {
        String::memset(pt, 0, sizeof(struct page_table));
    }
    __set_flags(&pt->pages[PT_INDEX(v)], flags);
    pt->pages[PT_INDEX(v)].address = p / 0x1000;
    return SUCCESS;
}
}
}
