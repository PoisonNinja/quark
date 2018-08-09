#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <arch/mm/mm.h>
#include <boot/info.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace
{
multiboot_memory_map_t *mmap = nullptr;
struct multiboot_tag *mmap_tag;
struct multiboot_fixed *multiboot = nullptr;
struct Boot::info *info;
}  // namespace

namespace Memory
{
namespace Physical
{
void init_early_alloc(struct Boot::info *b)
{
    addr_t highest = 0;
    info = b;
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
                for (auto tmp = mmap;
                     reinterpret_cast<multiboot_uint8_t *>(tmp) <
                     reinterpret_cast<multiboot_uint8_t *>(mmap_tag) +
                         mmap_tag->size;
                     tmp = reinterpret_cast<multiboot_memory_map_t *>(
                         reinterpret_cast<addr_t>(tmp) +
                         (reinterpret_cast<struct multiboot_tag_mmap *>(
                              mmap_tag))
                             ->entry_size)) {
                    // Look for the highest memory address (needed for buddy)
                    if (Memory::Virtual::align_up(tmp->addr + tmp->len) >
                        highest)
                        highest =
                            Memory::Virtual::align_up(tmp->addr + tmp->len);
                }
                break;
        }
    }
    info->highest = highest;
}

addr_t early_allocate()
{
    addr_t multiboot_start =
        Memory::Virtual::align_down(reinterpret_cast<addr_t>(multiboot) - VMA);
    addr_t multiboot_end =
        Memory::Virtual::align_up(multiboot_start + multiboot->total_size);
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
            for (addr_t i = Memory::Virtual::align_up(mmap->addr);
                 i < Memory::Virtual::align_up(mmap->addr) +
                         Memory::Virtual::align_down(mmap->len);
                 i += Memory::Virtual::PAGE_SIZE) {
                /*
                 * Unconditionally increment the start of this zone and
                 * decrement the size of the zone. This works because there are
                 * only two possible conditions: memory we can't use and memory
                 * we can. Both will not be evaluated again, so we can advance.
                 */
                mmap->addr += Memory::Virtual::PAGE_SIZE;
                mmap->len -= Memory::Virtual::PAGE_SIZE;
                // Check if it's in a restricted area
                if (!((i >= Memory::Virtual::align_down(info->kernel_start) &&
                       i < Memory::Virtual::align_up(info->kernel_end)) ||
                      (i >= multiboot_start && i < multiboot_end) ||
                      (i >= Memory::Virtual::align_down(info->initrd_start) &&
                       i < Memory::Virtual::align_up(info->initrd_end)))) {
                    return i;
                }
            }
        }
    }
    Kernel::panic("Out of memory!\n");
}
}  // namespace Physical
}  // namespace Memory