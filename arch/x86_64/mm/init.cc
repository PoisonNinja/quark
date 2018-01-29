#include <arch/kernel/multiboot2.h>
#include <arch/mm/mm.h>
#include <boot/info.h>
#include <kernel.h>
#include <mm/physical.h>

namespace Memory
{
void arch_init(struct Boot::info &info)
{
    Log::printk(Log::INFO, "Kernel: %p -> %p\n", info.kernel_start,
                info.kernel_end);
    struct multiboot_fixed *multiboot =
        reinterpret_cast<struct multiboot_fixed *>(info.architecture_data);
    struct multiboot_tag *tag;
    for (tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<addr_t>(multiboot) + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<multiboot_uint8_t *>(tag) +
             ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_memory_map_t *mmap;
                Log::printk(Log::INFO, "Memory map:\n");
                for (mmap = (reinterpret_cast<struct multiboot_tag_mmap *>(tag))
                                ->entries;
                     reinterpret_cast<multiboot_uint8_t *>(mmap) <
                     reinterpret_cast<multiboot_uint8_t *>(tag) + tag->size;
                     mmap = reinterpret_cast<multiboot_memory_map_t *>(
                         reinterpret_cast<addr_t>(mmap) +
                         (reinterpret_cast<struct multiboot_tag_mmap *>(tag))
                             ->entry_size)) {
                    Log::printk(Log::INFO,
                                "    Base = %p, Length = %p, "
                                "Type = 0x%x\n",
                                static_cast<addr_t>(mmap->addr),
                                static_cast<addr_t>(mmap->len),
                                static_cast<addr_t>(mmap->type));
                    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                        for (addr_t i = mmap->addr; i < mmap->addr + mmap->len;
                             i += Memory::Virtual::PAGE_SIZE) {
                            if (i >= Memory::Virtual::align_down(
                                         info.kernel_start) &&
                                i < Memory::Virtual::align_up(
                                        info.kernel_end)) {
                                Log::printk(Log::DEBUG, "        Rejected %p\n",
                                            i);
                            } else {
                                Memory::Physical::put(i);
                            }
                        }
                    }
                }
            } break;
        }
    }
}
}  // namespace Memory
