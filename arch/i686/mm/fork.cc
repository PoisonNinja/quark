#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
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
                     uint32_t pd_index)
{
    String::memset(new_pt, 0, sizeof(struct page_table));
    for (int i = 0; i < 1024; i++) {
        if (old_pt->pages[i].present) {
            String::memcpy(&new_pt->pages[i], &old_pt->pages[i],
                           sizeof(struct page));
            addr_t new_page = Memory::Physical::allocate();
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
            addr_t source = (pd_index << 22) | (i << 12);
            String::memcpy(fork_page_pointer, reinterpret_cast<void*>(source),
                           0x1000);
        }
    }
}

addr_t fork()
{
    // Fractal mapping address of original page directory
    struct page_table* old_pd = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::recursive_entry, Memory::X86::recursive_entry);
    /*
     * Fractal mapping address of original page directory
     * Notice that the first parameter is Memory::X86::copy_entry, while the
     * other is Memory::X86::recursive_entry? This is because while the address
     * of the new page directory is loaded into the Memory::X86::copy_entry slot
     * of the current page directory, the new page directory recursive mapping
     * is still Memory::X86::recursive_entry in itself.
     */
    struct page_table* new_pd = (struct page_table*)Memory::X86::decode_fractal(
        Memory::X86::copy_entry, Memory::X86::recursive_entry);
    // Allocate a new page for the page directory
    addr_t fork_pd_phys = Memory::Physical::allocate();

    /*
     * Map the new page directory into a page that we control, which we reserved
     * using fork_page. This is so we can set up the fractal mapping
     * for the new page directory.
     */
    Memory::Virtual::map(reinterpret_cast<addr_t>(fork_page_pointer),
                         fork_pd_phys, PAGE_WRITABLE);
    /*
     * Copy the entire page directory. However, only the kernel pages will
     * remain (e.g. pd_index(x) >= 768 (3 GB)), as we want to duplicate the user
     * pages. Essentially, the kernel pages will be linked together so any
     * changes in kernel space (such as malloc) will propogate to kernel space
     * in other threads
     */
    String::memcpy(fork_page_pointer, old_pd, sizeof(struct page_table));

    // Set up fractal mapping for the new page directory
    fork_page_pointer->pages[Memory::X86::recursive_entry].address =
        fork_pd_phys / 0x1000;

    // Load the new page directory into the Memory::X86::copy_entry slot of the
    // current page directory
    old_pd->pages[Memory::X86::copy_entry].present = 1;
    old_pd->pages[Memory::X86::copy_entry].writable = 1;
    old_pd->pages[Memory::X86::copy_entry].address = fork_pd_phys / 0x1000;

    Memory::X86Family::invlpg(reinterpret_cast<addr_t>(new_pd));

    // Copy only user pages (0 - 3GB)
    for (int i = 0; i < 768; i++) {
        if (old_pd->pages[i].present) {
            // Copy the page data
            String::memcpy(&new_pd->pages[i], &old_pd->pages[i],
                           sizeof(struct page));
            // Allocate a new page table
            addr_t new_pt = Memory::Physical::allocate();
            // Set the page address
            new_pd->pages[i].address = new_pt / 0x1000;
            __copy_pt_entry((struct page_table*)Memory::X86::decode_fractal(
                                Memory::X86::copy_entry, i),
                            (struct page_table*)Memory::X86::decode_fractal(
                                Memory::X86::recursive_entry, i),
                            i);
        }
    }
    return fork_pd_phys;
}
}  // namespace Virtual
}  // namespace Memory
