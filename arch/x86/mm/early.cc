#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <arch/mm/mm.h>
#include <boot/info.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace
{
multiboot_memory_map_t *mmap      = nullptr;
struct multiboot_tag *mmap_tag    = nullptr;
struct multiboot_fixed *multiboot = nullptr;
struct boot::info *info           = nullptr;
} // namespace

namespace memory
{
namespace physical
{
void init_early_alloc(struct boot::info *b)
{
    info           = b;
    multiboot =
        reinterpret_cast<struct multiboot_fixed *>(info->architecture_data);
    struct multiboot_tag *tag;
    // Iterate through tags to find memory tag
    for (tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<addr_t>(multiboot) + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<multiboot_uint8_t *>(tag) +
             ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_MMAP:
                // Save tag for early memory allocator
                mmap_tag = tag;
                // Save start of mmap for early memory allocator
                mmap = (reinterpret_cast<struct multiboot_tag_mmap *>(tag))
                           ->entries;
                break;
        }
    }
}

addr_t early_allocate()
{
    if (!info) {
        kernel::panic("Attempted to allocate memory without initializing the "
                      "early memory allocator!\n");
    }
    /*
     * Iterate through the memory ranges. mmap will be set to the first free
     * range, whether set by the initial set up or incremented in the loop
     * below.
     */
    for (; reinterpret_cast<multiboot_uint8_t *>(mmap) <
           reinterpret_cast<multiboot_uint8_t *>(mmap_tag) + mmap_tag->size;
         mmap = reinterpret_cast<multiboot_memory_map_t *>(
             reinterpret_cast<addr_t>(mmap) +
             (reinterpret_cast<struct multiboot_tag_mmap *>(mmap_tag))
                 ->entry_size)) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            for (addr_t i = memory::virt::align_up(mmap->addr);
                 i < memory::virt::align_up(mmap->addr) +
                         memory::virt::align_down(mmap->len);
                 i += memory::virt::PAGE_SIZE) {
                /*
                 * Unconditionally increment the start of this zone and
                 * decrement the size of the zone. This works because there are
                 * only two possible conditions: memory we can't use and memory
                 * we can. Both will not be evaluated again, so we can advance.
                 */
                mmap->addr += memory::virt::PAGE_SIZE;
                mmap->len -= memory::virt::PAGE_SIZE;
                // Check if it's in a restricted area
                if (memory::x86::is_valid_physical_memory(i, *info)) {
                    return i;
                }
            }
        }
    }
    kernel::panic("Out of memory!\n");
}
} // namespace physical
} // namespace memory
