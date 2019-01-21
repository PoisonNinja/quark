#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <mm/physical.h>

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

bool unmap(addr_t v)
{
#ifdef X86_64
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
    if (!pml4->pages[memory::x86::pml4_index(v)].present ||
        !pdpt->pages[memory::x86::pdpt_index(v)].present ||
        !pd->pages[memory::x86::pd_index(v)].present ||
        !pt->pages[memory::x86::pt_index(v)].present)
        return false;
#else
    struct page_table* pd = (struct page_table*)memory::x86::decode_fractal(
        memory::x86::recursive_entry, memory::x86::recursive_entry);
    struct page_table* pt = (struct page_table*)memory::x86::decode_fractal(
        memory::x86::recursive_entry, memory::x86::pd_index(v));
    if (!pd->pages[memory::x86::pd_index(v)].present ||
        !pt->pages[memory::x86::pt_index(v)].present)
        return false;
#endif
    if (!pt->pages[memory::x86::pt_index(v)].hardware) {
        memory::physical::free(pt->pages[memory::x86::pt_index(v)].address *
                               0x1000);
        memory::x86::invlpg(reinterpret_cast<addr_t>(v));
    }
    pt->pages[memory::x86::pt_index(v)].present = 0;
    if (__table_is_empty(pt)) {
        memory::physical::free(pd->pages[memory::x86::pd_index(v)].address *
                               0x1000);
        pd->pages[memory::x86::pd_index(v)].present = 0;
        /*
         * TODO: This unconditionally invalidates the mapping even if
         * nothing changed. Perhaps, we should only do this if the mapping or
         * flags changed.
         */
        memory::x86::invlpg(reinterpret_cast<addr_t>(pt));
#ifdef X86_64
        if (__table_is_empty(pd)) {
            memory::physical::free(
                pdpt->pages[memory::x86::pdpt_index(v)].address * 0x1000);
            pdpt->pages[memory::x86::pdpt_index(v)].present = 0;
            memory::x86::invlpg(reinterpret_cast<addr_t>(pd));
            if (__table_is_empty(pdpt)) {
                memory::physical::free(
                    pml4->pages[memory::x86::pml4_index(v)].address * 0x1000);
                pml4->pages[memory::x86::pml4_index(v)].present = 0;
                memory::x86::invlpg(reinterpret_cast<addr_t>(pdpt));
            }
        }
#endif
    }
    return true;
}
} // namespace virt
} // namespace memory
