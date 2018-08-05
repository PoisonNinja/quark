#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
bool test(addr_t v)
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
    return true;
}
}  // namespace Virtual
}  // namespace Memory
