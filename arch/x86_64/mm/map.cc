#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace memory
{
namespace virt
{
static inline int __set_address(struct memory::virt::page* page)
{
    if (!page->present) {
        page->address = memory::physical::allocate() / 0x1000;
        return 1;
    }
    return 0;
}

static inline void __set_flags(struct page* page, uint8_t flags)
{
    page->present  = 1;
    page->writable = (flags & PAGE_WRITABLE) ? 1 : 0;
    page->user     = (flags & PAGE_USER) ? 1 : 0;
    page->global   = (flags & PAGE_GLOBAL) ? 1 : 0;
    page->cow      = (flags & PAGE_COW) ? 1 : 0;
    page->hardware = (flags & PAGE_HARDWARE) ? 1 : 0;
    page->nx       = (flags & PAGE_NX) ? 1 : 0;
}

bool map(addr_t v, addr_t p, int flags)
{
    struct page_table* pml4 = (struct page_table*)memory::x86::decode_fractal(
        memory::x86::recursive_entry, memory::x86::recursive_entry,
        memory::x86::recursive_entry, memory::x86::recursive_entry);
    struct page_table* pdpt = (struct page_table*)memory::x86::decode_fractal(
        memory::x86::recursive_entry, memory::x86::recursive_entry,
        memory::x86::recursive_entry, memory::x86::pml4_index(v));
    struct page_table* pd = (struct page_table*)memory::x86::decode_fractal(
        memory::x86::recursive_entry, memory::x86::recursive_entry,
        memory::x86::pml4_index(v), memory::x86::pdpt_index(v));
    struct page_table* pt = (struct page_table*)memory::x86::decode_fractal(
        memory::x86::recursive_entry, memory::x86::pml4_index(v),
        memory::x86::pdpt_index(v), memory::x86::pd_index(v));
    int r = 0;
    r     = __set_address(&pml4->pages[memory::x86::pml4_index(v)]);
    __set_flags(&pml4->pages[memory::x86::pml4_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    /*
     * TODO: This unconditionally invalidates the mapping even if
     * nothing changed. Perhaps, we should only do this if the mapping or
     * flags changed.
     */
    memory::x86::invlpg(reinterpret_cast<addr_t>(pdpt));
    /*
     * __set_address returns whether it allocated memory or not.
     * If it did, we need to memset it ourselves to 0.
     */
    if (r) {
        libcxx::memset(pdpt, 0, sizeof(struct page_table));
    }
    r = __set_address(&pdpt->pages[memory::x86::pdpt_index(v)]);
    __set_flags(&pdpt->pages[memory::x86::pdpt_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    memory::x86::invlpg(reinterpret_cast<addr_t>(pd));
    if (r) {
        libcxx::memset(pd, 0, sizeof(struct page_table));
    }
    r = __set_address(&pd->pages[memory::x86::pd_index(v)]);
    __set_flags(&pd->pages[memory::x86::pd_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    memory::x86::invlpg(reinterpret_cast<addr_t>(pt));
    if (r) {
        libcxx::memset(pt, 0, sizeof(struct page_table));
    }
    pt->pages[memory::x86::pt_index(v)].address = p / 0x1000;
    __set_flags(&pt->pages[memory::x86::pt_index(v)], flags);
    memory::x86::invlpg(v);
    return true;
}
} // namespace virt
} // namespace memory
