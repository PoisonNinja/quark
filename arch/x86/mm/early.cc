#include <arch/kernel/multiboot2.h>
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

#ifdef X86_64
constexpr addr_t vma = 0xFFFFFFFF80000000;
#else
constexpr addr_t vma = 0xC0000000;
#endif
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
    for (tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<addr_t>(multiboot) + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<multiboot_uint8_t *>(tag) +
             ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_MMAP:
                mmap_tag = tag;
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
        Memory::Virtual::align_down(reinterpret_cast<addr_t>(multiboot) - vma);
    addr_t multiboot_end =
        Memory::Virtual::align_up(multiboot_start + multiboot->total_size);
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
                mmap->addr += Memory::Virtual::PAGE_SIZE,
                    mmap->len -= Memory::Virtual::PAGE_SIZE;
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