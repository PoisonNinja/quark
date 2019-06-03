#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace memory
{
namespace virt
{
static inline bool __table_is_empty(struct page_table* table)
{
    for (size_t i = 0; i < PAGE_SIZE / sizeof(struct page); i++) {
        if (table->pages[i].present)
            return false;
    }
    return true;
}

bool unmap(addr_t v, int flags)
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
    pt->pages[memory::x86_64::pt_index(v)].present = 0;
    if (flags & UNMAP_FREE) {
        memory::physical::free(pt->pages[memory::x86_64::pt_index(v)].address *
                               0x1000);
    }
    memory::x86_64::invlpg(reinterpret_cast<addr_t>(v));
    if (__table_is_empty(pt)) {
        pd->pages[memory::x86_64::pd_index(v)].present = 0;
        memory::physical::free(pd->pages[memory::x86_64::pd_index(v)].address *
                               0x1000);
        /*
         * TODO: This unconditionally invalidates the mapping even if
         * nothing changed. Perhaps, we should only do this if the mapping or
         * flags changed.
         */
        memory::x86_64::invlpg(reinterpret_cast<addr_t>(pt));
        if (__table_is_empty(pd)) {
            pdpt->pages[memory::x86_64::pdpt_index(v)].present = 0;
            memory::physical::free(
                pdpt->pages[memory::x86_64::pdpt_index(v)].address * 0x1000);
            memory::x86_64::invlpg(reinterpret_cast<addr_t>(pd));
            if (__table_is_empty(pdpt)) {
                pml4->pages[memory::x86_64::pml4_index(v)].present = 0;
                memory::physical::free(
                    pml4->pages[memory::x86_64::pml4_index(v)].address *
                    0x1000);
                memory::x86_64::invlpg(reinterpret_cast<addr_t>(pdpt));
            }
        }
    }
    return true;
}
} // namespace virt
} // namespace memory
