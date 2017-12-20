#include <arch/mm/mm.h>
#include <kernel.h>
#include <mm/virtual.h>
#include <string.h>

namespace Memory
{
namespace Virtual
{
static inline int __set_address(struct Memory::page* page)
{
    if (!page->present) {
        Kernel::panic(
            "Page not present, and we don't have page allocation support "
            "yet!\n");
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
    struct page_table* pml4 = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    struct page_table* pdpt = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(v));
    struct page_table* pd = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, RECURSIVE_ENTRY, PML4_INDEX(v), PDPT_INDEX(v));
    struct page_table* pt = (struct page_table*)entry_to_address(
        RECURSIVE_ENTRY, PML4_INDEX(v), PDPT_INDEX(v), PD_INDEX(v));
    int r = 0;
    r = __set_address(&pml4->pages[PML4_INDEX(v)]);
    __set_flags(&pml4->pages[PML4_INDEX(v)], flags);
    /*
     * __set_address returns whether it allocated memory or not.
     * If it did, we need to memset it ourselves to 0.
     */
    if (r) {
        memset(pdpt, 0, sizeof(struct page_table));
    }
    r = __set_address(&pdpt->pages[PDPT_INDEX(v)]);
    __set_flags(&pdpt->pages[PDPT_INDEX(v)], flags);
    if (r) {
        memset(pd, 0, sizeof(struct page_table));
    }
    r = __set_address(&pd->pages[PD_INDEX(v)]);
    __set_flags(&pd->pages[PD_INDEX(v)], flags);
    if (r) {
        memset(pt, 0, sizeof(struct page_table));
    }
    __set_flags(&pt->pages[PT_INDEX(v)], flags);
    pt->pages[PT_INDEX(v)].address = p / 0x1000;
    return r;
}
}  // namespace Virtual
}  // namespace Memory
