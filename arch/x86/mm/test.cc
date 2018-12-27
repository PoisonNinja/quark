#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace memory
{
namespace Virtual
{
bool test(addr_t v)
{
#ifdef X86_64
    struct page_table* pml4 = (struct page_table*)memory::X86::decode_fractal(
        memory::X86::recursive_entry, memory::X86::recursive_entry,
        memory::X86::recursive_entry, memory::X86::recursive_entry);
    struct page_table* pdpt = (struct page_table*)memory::X86::decode_fractal(
        memory::X86::recursive_entry, memory::X86::recursive_entry,
        memory::X86::recursive_entry, memory::X86::pml4_index(v));
    struct page_table* pd = (struct page_table*)memory::X86::decode_fractal(
        memory::X86::recursive_entry, memory::X86::recursive_entry,
        memory::X86::pml4_index(v), memory::X86::pdpt_index(v));
    struct page_table* pt = (struct page_table*)memory::X86::decode_fractal(
        memory::X86::recursive_entry, memory::X86::pml4_index(v),
        memory::X86::pdpt_index(v), memory::X86::pd_index(v));
    if (!pml4->pages[memory::X86::pml4_index(v)].present ||
        !pdpt->pages[memory::X86::pdpt_index(v)].present ||
        !pd->pages[memory::X86::pd_index(v)].present ||
        !pt->pages[memory::X86::pt_index(v)].present)
        return false;
#else
    struct page_table* pd = (struct page_table*)memory::X86::decode_fractal(
        memory::X86::recursive_entry, memory::X86::recursive_entry);
    struct page_table* pt = (struct page_table*)memory::X86::decode_fractal(
        memory::X86::recursive_entry, memory::X86::pd_index(v));
    if (!pd->pages[memory::X86::pd_index(v)].present ||
        !pt->pages[memory::X86::pt_index(v)].present)
        return false;
#endif
    return true;
}
}  // namespace Virtual
}  // namespace memory
