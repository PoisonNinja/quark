#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <boot/info.h>
#include <kernel.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace memory
{
void arch_init(struct Boot::info &info)
{
    struct multiboot_fixed *multiboot =
        reinterpret_cast<struct multiboot_fixed *>(info.architecture_data);
    addr_t multiboot_start =
        memory::virt::align_down(reinterpret_cast<addr_t>(multiboot) - VMA);
    addr_t multiboot_end = memory::virt::align_up(
        reinterpret_cast<addr_t>(multiboot) - VMA + multiboot->total_size);
    Log::printk(Log::LogLevel::INFO, "Restricted memory areas:\n");
    Log::printk(Log::LogLevel::INFO, "    Kernel:    [%p - %p]\n",
                info.kernel_start, info.kernel_end);
    Log::printk(Log::LogLevel::INFO, "    Multiboot: [%p - %p]\n",
                multiboot_start, multiboot_end);
    Log::printk(Log::LogLevel::INFO, "    initrd:    [%p - %p]\n",
                info.initrd_start, info.initrd_end);
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
                Log::printk(Log::LogLevel::INFO, "Memory map:\n");
                for (mmap = (reinterpret_cast<struct multiboot_tag_mmap *>(tag))
                                ->entries;
                     reinterpret_cast<multiboot_uint8_t *>(mmap) <
                     reinterpret_cast<multiboot_uint8_t *>(tag) + tag->size;
                     mmap = reinterpret_cast<multiboot_memory_map_t *>(
                         reinterpret_cast<addr_t>(mmap) +
                         (reinterpret_cast<struct multiboot_tag_mmap *>(tag))
                             ->entry_size)) {
                    Log::printk(Log::LogLevel::INFO,
                                "    [%p - %p] Type: 0x%x\n",
                                static_cast<addr_t>(mmap->addr),
                                static_cast<addr_t>(mmap->addr) +
                                    static_cast<addr_t>(mmap->len),
                                static_cast<addr_t>(mmap->type));
                    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                        for (addr_t i = mmap->addr; i < mmap->addr + mmap->len;
                             i += memory::virt::PAGE_SIZE) {
                            if (memory::X86::is_valid_physical_memory(i,
                                                                      info)) {
                                memory::physical::free(i);
                            }
                        }
                    }
                }
            } break;
        }
    }
} // namespace memory
} // namespace memory
