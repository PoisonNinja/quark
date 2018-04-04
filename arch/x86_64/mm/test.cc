#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
bool arch_test(addr_t v)
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
    return true;
}
}  // namespace Virtual
}  // namespace Memory
