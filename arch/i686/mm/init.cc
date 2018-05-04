#include <arch/kernel/multiboot2.h>
#include <arch/mm/virtual.h>
#include <boot/info.h>
#include <kernel.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace Memory
{
void arch_init(struct Boot::info &info)
{
    struct multiboot_fixed *multiboot =
        reinterpret_cast<struct multiboot_fixed *>(info.architecture_data);
    addr_t multiboot_start = Memory::Virtual::align_down(
        reinterpret_cast<addr_t>(multiboot) - 0xC0000000);
    addr_t multiboot_end =
        Memory::Virtual::align_up(multiboot_start + multiboot->total_size);
    Log::printk(Log::INFO, "Restricted memory areas:\n");
    Log::printk(Log::INFO, "Kernel: %p -> %p\n", info.kernel_start,
                info.kernel_end);
    Log::printk(Log::INFO, "Multiboot: %p -> %p\n", multiboot_start,
                multiboot_end);
    Log::printk(Log::INFO, "initrd: %p -> %p\n", info.initrd_start,
                info.initrd_end);
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
                                Log::printk(
                                    Log::DEBUG,
                                    "        Rejected %p because in kernel\n",
                                    i);
                            } else if (i >= multiboot_start &&
                                       i < multiboot_end) {
                                Log::printk(Log::DEBUG,
                                            "        Rejected %p "
                                            "because in "
                                            "multiboot\n",
                                            i);
                            } else if (i >= Memory::Virtual::align_down(
                                                info.initrd_start) &&
                                       i < Memory::Virtual::align_up(
                                               info.initrd_end)) {
                                Log::printk(Log::DEBUG,
                                            "        Rejected %p because "
                                            "in initrd\n",
                                            i);
                            } else {
                                Memory::Physical::free(i);
                            }
                        }
                    }
                }
            } break;
        }
    }
}
}  // namespace Memory
