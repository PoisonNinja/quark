#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
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
    page->cow = (flags & PAGE_COW) ? 1 : 0;
    page->hardware = (flags & PAGE_HARDWARE) ? 1 : 0;
}

bool arch_protect(addr_t v, int flags)
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
    __set_flags(&pml4->pages[Memory::X64::pml4_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    __set_flags(&pdpt->pages[Memory::X64::pdpt_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    __set_flags(&pd->pages[Memory::X64::pd_index(v)],
                PAGE_WRITABLE | ((flags & PAGE_USER) ? PAGE_USER : 0));
    __set_flags(&pt->pages[Memory::X64::pt_index(v)], flags);
    Memory::X64::invlpg(v);
    return true;
}
}  // namespace Virtual
}  // namespace Memory
