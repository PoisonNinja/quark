#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
static inline void __set_flags(struct page* page, uint8_t flags)
{
    page->present  = 1;
    page->writable = (flags & PAGE_WRITABLE) ? 1 : 0;
    page->user     = (flags & PAGE_USER) ? 1 : 0;
    page->global   = (flags & PAGE_GLOBAL) ? 1 : 0;
    page->cow      = (flags & PAGE_COW) ? 1 : 0;
    page->hardware = (flags & PAGE_HARDWARE) ? 1 : 0;
#ifdef X86_64
    page->nx = (flags & PAGE_NX) ? 1 : 0;
#endif
}

bool protect(addr_t v, int flags)
{
#ifdef X86_64
    struct page_table* pml4 = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::recursive_entry,
        Memory::X86::recursive_entry, Memory::X86::recursive_entry);
    struct page_table* pdpt = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::recursive_entry,
        Memory::X86::recursive_entry, Memory::X86::pml4_index(v));
    struct page_table* pd = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::recursive_entry,
        Memory::X86::pml4_index(v), Memory::X86::pdpt_index(v));
    struct page_table* pt = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::pml4_index(v),
        Memory::X86::pdpt_index(v), Memory::X86::pd_index(v));
    if (!pml4->pages[Memory::X86::pml4_index(v)].present ||
        !pdpt->pages[Memory::X86::pdpt_index(v)].present ||
        !pd->pages[Memory::X86::pd_index(v)].present ||
        !pt->pages[Memory::X86::pt_index(v)].present)
        return false;
#else
    struct page_table* pd = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::recursive_entry);
    struct page_table* pt = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::pd_index(v));
    if (!pd->pages[Memory::X86::pd_index(v)].present ||
        !pt->pages[Memory::X86::pt_index(v)].present)
        return false;
#endif
#ifdef X86_64
    __set_flags(&pml4->pages[Memory::X86::pml4_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    /*
     * TODO: This unconditionally invalidates the mapping even if
     * nothing changed. Perhaps, we should only do this if the mapping or
     * flags changed.
     */
    Memory::X86::invlpg(reinterpret_cast<addr_t>(pdpt));
    __set_flags(&pdpt->pages[Memory::X86::pdpt_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    Memory::X86::invlpg(reinterpret_cast<addr_t>(pd));
#endif
    __set_flags(&pd->pages[Memory::X86::pd_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    Memory::X86::invlpg(reinterpret_cast<addr_t>(pt));
    __set_flags(&pt->pages[Memory::X86::pt_index(v)], flags);
    Memory::X86::invlpg(v);
    return true;
}
} // namespace Virtual
} // namespace Memory
