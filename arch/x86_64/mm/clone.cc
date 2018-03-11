#include <arch/mm/mm.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
namespace Virtual
{
static struct page_table __attribute__((aligned(0x1000))) fork_page;
struct page_table* const fork_page_pointer = &fork_page;

void __copy_pt_entry(struct page_table* new_pt, struct page_table* old_pt,
                     uint64_t pml4_index, uint64_t pdpt_index,
                     uint64_t pd_index)
{
    String::memset(new_pt, 0, sizeof(struct page_table));
    for (int i = 0; i < 512; i++) {
        if (old_pt->pages[i].present) {
            String::memcpy(&new_pt->pages[i], &old_pt->pages[i],
                           sizeof(struct page));
            addr_t new_page = Memory::Physical::get();
            new_pt->pages[i].address = new_page / 0x1000;
            /*
             * Map the new page into a page that we control so we can copy
             * the contents of the page
             */
            Memory::Virtual::map(reinterpret_cast<addr_t>(fork_page_pointer),
                                 new_page, PAGE_WRITABLE);
            /*
             * Calculate the virtual address given the indexes for the source
             */
            uint64_t source = ((pml4_index << 39) | (pdpt_index << 30) |
                               (pd_index << 21) | (i << 12));
            String::memcpy(fork_page_pointer, reinterpret_cast<void*>(source),
                           sizeof(struct page));
        }
    }
}

void __copy_pd_entry(struct page_table* new_pd, struct page_table* old_pd,
                     int pml4_index, int pdpt_index)
{
    String::memset(new_pd, 0, sizeof(struct page_table));
    for (int i = 0; i < 512; i++) {
        if (old_pd->pages[i].present) {
            String::memcpy(&new_pd->pages[i], &old_pd->pages[i],
                           sizeof(struct page));
            addr_t new_pt = Memory::Physical::get();
            new_pd->pages[i].address = new_pt / 0x1000;
            __copy_pt_entry((struct page_table*)Memory::X64::decode_fractal(
                                COPY_ENTRY, pml4_index, pdpt_index, i),
                            (struct page_table*)Memory::X64::decode_fractal(
                                RECURSIVE_ENTRY, pml4_index, pdpt_index, i),
                            pml4_index, pdpt_index, i);
        }
    }
}

void __copy_pdpt_entry(struct page_table* new_pdpt, struct page_table* old_pdpt,
                       int pml4_index)
{
    String::memset(new_pdpt, 0, sizeof(struct page_table));
    for (int i = 0; i < 512; i++) {
        if (old_pdpt->pages[i].present) {
            String::memcpy(&new_pdpt->pages[i], &old_pdpt->pages[i],
                           sizeof(struct page));
            addr_t new_pd = Memory::Physical::get();
            new_pdpt->pages[i].address = new_pd / 0x1000;
            __copy_pd_entry(
                (struct page_table*)Memory::X64::decode_fractal(
                    COPY_ENTRY, RECURSIVE_ENTRY, pml4_index, i),
                (struct page_table*)Memory::X64::decode_fractal(
                    RECURSIVE_ENTRY, RECURSIVE_ENTRY, pml4_index, i),
                pml4_index, i);
        }
    }
}

addr_t arch_clone()
{
    // Fractal mapping address of original PML4
    struct page_table* old_pml4 =
        (struct page_table*)Memory::X64::decode_fractal(
            RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    /*
     * Fractal mapping address of original PML4
     * Notice that the first parameter is COPY_ENTRY, while the rest are
     * RECURSIVE_ENTRY. This is because while the address of the new PML4
     * is loaded into the COPY_ENTRY slot of the current PML4, the new
     * PML4 recursive mapping is still RECURSIVE_ENTRY in itself.
     */
    struct page_table* new_pml4 =
        (struct page_table*)Memory::X64::decode_fractal(
            COPY_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY);
    // Allocate a new page for the PML4
    addr_t fork_pml4_phys = Memory::Physical::get();
    /*
     * Map the new PML4 into a page that we control, which we reserved
     * using fork_page. This is so we can set up the fractal mapping
     * for the new PML4.
     */
    Memory::Virtual::map(reinterpret_cast<addr_t>(fork_page_pointer),
                         fork_pml4_phys, PAGE_WRITABLE);
    /*
     * Copy the entire PML4. However, only the kernel pages will remain
     * (e.g. PML4_INDEX(x) >= 256), as we want to duplicate the user
     * pages
     */
    String::memcpy(fork_page_pointer, old_pml4, sizeof(struct page_table));

    // Set up fractal mapping for the new PML4
    fork_page_pointer->pages[RECURSIVE_ENTRY].address = fork_pml4_phys / 0x1000;

    // Load the new PML4 into the COPY_ENTRY slot of the current PML4
    old_pml4->pages[COPY_ENTRY].present = 1;
    old_pml4->pages[COPY_ENTRY].writable = 1;
    old_pml4->pages[COPY_ENTRY].address = fork_pml4_phys / 0x1000;

    // Flush the entire TLB by reloading cr3
    // TODO: Use invlpg
    Memory::X64::write_cr3(Memory::X64::read_cr3());

    // Copy only user pages
    for (int i = 0; i < 256; i++) {
        if (old_pml4->pages[i].present) {
            // Copy the page data
            String::memcpy(&new_pml4->pages[i], &old_pml4->pages[i],
                           sizeof(struct page));
            // Allocate a new PDPT
            addr_t new_pdpt = Memory::Physical::get();
            // Set the page address
            new_pml4->pages[i].address = new_pdpt / 0x1000;
            __copy_pdpt_entry(
                (struct page_table*)Memory::X64::decode_fractal(
                    COPY_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, i),
                (struct page_table*)Memory::X64::decode_fractal(
                    RECURSIVE_ENTRY, RECURSIVE_ENTRY, RECURSIVE_ENTRY, i),
                i);
        }
    }
    return fork_pml4_phys;
}
}
}
