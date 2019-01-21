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
static struct page_table __attribute__((aligned(0x1000))) fork_page;
struct page_table* const fork_page_pointer = &fork_page;

void __copy_pt_entry(struct page_table* new_pt, struct page_table* old_pt,
                     uint64_t pml4_index, uint64_t pdpt_index,
                     uint64_t pd_index)
{
    libcxx::memset(new_pt, 0, sizeof(struct page_table));
    for (size_t i = 0; i < PAGE_SIZE / sizeof(struct page); i++) {
        if (old_pt->pages[i].present) {
            libcxx::memcpy(&new_pt->pages[i], &old_pt->pages[i],
                           sizeof(struct page));
            addr_t new_page          = memory::physical::allocate();
            new_pt->pages[i].address = new_page / 0x1000;
            /*
             * Map the new page into a page that we control so we can copy
             * the contents of the page
             */
            memory::virt::map(reinterpret_cast<addr_t>(fork_page_pointer),
                              new_page, PAGE_WRITABLE);
            /*
             * Calculate the virtual address given the indexes for the source
             */
            addr_t source = ((pml4_index << 39) | (pdpt_index << 30) |
                             (pd_index << 21) | (i << 12));
            libcxx::memcpy(fork_page_pointer, reinterpret_cast<void*>(source),
                           0x1000);
        }
    }
}

void __copy_pt_entry(struct page_table* new_pt, struct page_table* old_pt,
                     uint32_t pd_index)
{
    libcxx::memset(new_pt, 0, sizeof(struct page_table));
    for (size_t i = 0; i < PAGE_SIZE / sizeof(struct page); i++) {
        if (old_pt->pages[i].present) {
            libcxx::memcpy(&new_pt->pages[i], &old_pt->pages[i],
                           sizeof(struct page));
            addr_t new_page          = memory::physical::allocate();
            new_pt->pages[i].address = new_page / 0x1000;
            /*
             * Map the new page into a page that we control so we can copy
             * the contents of the page
             */
            memory::virt::map(reinterpret_cast<addr_t>(fork_page_pointer),
                              new_page, PAGE_WRITABLE);
            /*
             * Calculate the virtual address given the indexes for the source
             */
            addr_t source = (pd_index << 22) | (i << 12);
            libcxx::memcpy(fork_page_pointer, reinterpret_cast<void*>(source),
                           0x1000);
        }
    }
}

void __copy_pd_entry(struct page_table* new_pd, struct page_table* old_pd,
                     int pml4_index, int pdpt_index)
{
    libcxx::memset(new_pd, 0, sizeof(struct page_table));
    for (size_t i = 0; i < PAGE_SIZE / sizeof(struct page); i++) {
        if (old_pd->pages[i].present) {
            libcxx::memcpy(&new_pd->pages[i], &old_pd->pages[i],
                           sizeof(struct page));
            addr_t new_pt            = memory::physical::allocate();
            new_pd->pages[i].address = new_pt / 0x1000;
            __copy_pt_entry(
                (struct page_table*)memory::x86::decode_fractal(
                    memory::x86::copy_entry, pml4_index, pdpt_index, i),
                (struct page_table*)memory::x86::decode_fractal(
                    memory::x86::recursive_entry, pml4_index, pdpt_index, i),
                pml4_index, pdpt_index, i);
        }
    }
}

void __copy_pdpt_entry(struct page_table* new_pdpt, struct page_table* old_pdpt,
                       int pml4_index)
{
    libcxx::memset(new_pdpt, 0, sizeof(struct page_table));
    for (size_t i = 0; i < 512; i++) {
        if (old_pdpt->pages[i].present) {
            libcxx::memcpy(&new_pdpt->pages[i], &old_pdpt->pages[i],
                           sizeof(struct page));
            addr_t new_pd              = memory::physical::allocate();
            new_pdpt->pages[i].address = new_pd / 0x1000;
            __copy_pd_entry((struct page_table*)memory::x86::decode_fractal(
                                memory::x86::copy_entry,
                                memory::x86::recursive_entry, pml4_index, i),
                            (struct page_table*)memory::x86::decode_fractal(
                                memory::x86::recursive_entry,
                                memory::x86::recursive_entry, pml4_index, i),
                            pml4_index, i);
        }
    }
}

addr_t fork()
{
    // Fractal mapping address of original PML4
#ifdef X86_64
    struct page_table* old_pml4 =
        (struct page_table*)memory::x86::decode_fractal(
            memory::x86::recursive_entry, memory::x86::recursive_entry,
            memory::x86::recursive_entry, memory::x86::recursive_entry);
#else
    struct page_table* old_pml4 =
        (struct page_table*)memory::x86::decode_fractal(
            memory::x86::recursive_entry, memory::x86::recursive_entry);
#endif
    /*
     * Fractal mapping address of original PML4
     * Notice that the first parameter is memory::x86::copy_entry, while the
     * rest are memory::x86::recursive_entry. This is because while the address
     * of the new PML4 is loaded into the memory::x86::copy_entry slot of the
     * current PML4, the new PML4 recursive mapping is still
     * memory::x86::recursive_entry in itself.
     */
#ifdef X86_64
    struct page_table* new_pml4 =
        (struct page_table*)memory::x86::decode_fractal(
            memory::x86::copy_entry, memory::x86::recursive_entry,
            memory::x86::recursive_entry, memory::x86::recursive_entry);
#else
    struct page_table* new_pml4 =
        (struct page_table*)memory::x86::decode_fractal(
            memory::x86::copy_entry, memory::x86::recursive_entry);
#endif
    // Allocate a new page for the PML4
    addr_t fork_pml4_phys = memory::physical::allocate();
    /*
     * Map the new PML4 into a page that we control, which we reserved
     * using fork_page. This is so we can set up the fractal mapping
     * for the new PML4.
     */
    memory::virt::map(reinterpret_cast<addr_t>(fork_page_pointer),
                      fork_pml4_phys, PAGE_WRITABLE);
    /*
     * Copy the entire PML4. However, only the kernel pages will remain
     * (e.g. pml4_index(x) >= 256), as we want to duplicate the user pages
     */
    libcxx::memcpy(fork_page_pointer, old_pml4, sizeof(struct page_table));

    // Set up fractal mapping for the new PML4
    fork_page_pointer->pages[memory::x86::recursive_entry].address =
        fork_pml4_phys / 0x1000;

    // Load the new PML4 into the memory::x86::copy_entry slot of the current
    // PML4
    old_pml4->pages[memory::x86::copy_entry].present  = 1;
    old_pml4->pages[memory::x86::copy_entry].writable = 1;
    old_pml4->pages[memory::x86::copy_entry].address  = fork_pml4_phys / 0x1000;

    memory::x86::invlpg(reinterpret_cast<addr_t>(new_pml4));

#ifdef X86_64
    // Copy only user pages (lower half)
    for (int i = 0; i < 256; i++) {
#else
    // Copy only user pages (0 - 3GB)
    for (int i = 0; i < 768; i++) {
#endif
        if (old_pml4->pages[i].present) {
            // Copy the page data
            libcxx::memcpy(&new_pml4->pages[i], &old_pml4->pages[i],
                           sizeof(struct page));
            // Allocate a new PDPT
            addr_t new_pdpt = memory::physical::allocate();
            // Set the page address
            new_pml4->pages[i].address = new_pdpt / 0x1000;
#ifdef X86_64
            __copy_pdpt_entry(
                (struct page_table*)memory::x86::decode_fractal(
                    memory::x86::copy_entry, memory::x86::recursive_entry,
                    memory::x86::recursive_entry, i),
                (struct page_table*)memory::x86::decode_fractal(
                    memory::x86::recursive_entry, memory::x86::recursive_entry,
                    memory::x86::recursive_entry, i),
                i);
#else
            __copy_pt_entry((struct page_table*)memory::x86::decode_fractal(
                                memory::x86::copy_entry, i),
                            (struct page_table*)memory::x86::decode_fractal(
                                memory::x86::recursive_entry, i),
                            i);
#endif
        }
    }
    return fork_pml4_phys;
}
} // namespace virt
} // namespace memory
