#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace memory
{
namespace virt
{
bool test(addr_t v)
{
    struct page_table* pml4 =
        (struct page_table*)memory::x86_64::decode_fractal(
            memory::x86_64::recursive_entry, memory::x86_64::recursive_entry,
            memory::x86_64::recursive_entry, memory::x86_64::recursive_entry);
    struct page_table* pdpt =
        (struct page_table*)memory::x86_64::decode_fractal(
            memory::x86_64::recursive_entry, memory::x86_64::recursive_entry,
            memory::x86_64::recursive_entry, memory::x86_64::pml4_index(v));
    struct page_table* pd = (struct page_table*)memory::x86_64::decode_fractal(
        memory::x86_64::recursive_entry, memory::x86_64::recursive_entry,
        memory::x86_64::pml4_index(v), memory::x86_64::pdpt_index(v));
    struct page_table* pt = (struct page_table*)memory::x86_64::decode_fractal(
        memory::x86_64::recursive_entry, memory::x86_64::pml4_index(v),
        memory::x86_64::pdpt_index(v), memory::x86_64::pd_index(v));
    if (!pml4->pages[memory::x86_64::pml4_index(v)].present ||
        !pdpt->pages[memory::x86_64::pdpt_index(v)].present ||
        !pd->pages[memory::x86_64::pd_index(v)].present ||
        !pt->pages[memory::x86_64::pt_index(v)].present)
        return false;
    return true;
}
} // namespace virt
} // namespace memory
