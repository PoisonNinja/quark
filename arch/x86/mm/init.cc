#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <boot/info.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace memory
{
void arch_init(struct boot::info &info)
{
    struct multiboot_fixed *multiboot =
        reinterpret_cast<struct multiboot_fixed *>(info.architecture_data);
    addr_t multiboot_start =
        memory::virt::align_down(reinterpret_cast<addr_t>(multiboot) - VMA);
    addr_t multiboot_end = memory::virt::align_up(
        reinterpret_cast<addr_t>(multiboot) - VMA + multiboot->total_size);
    log::printk(log::log_level::INFO, "Restricted memory areas:\n");
    log::printk(log::log_level::INFO, "    Kernel:    [%p - %p]\n",
                info.kernel_start, info.kernel_end);
    log::printk(log::log_level::INFO, "    Multiboot: [%p - %p]\n",
                multiboot_start, multiboot_end);
    log::printk(log::log_level::INFO, "    initrd:    [%p - %p]\n",
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
                log::printk(log::log_level::INFO, "Memory map:\n");
                for (mmap = (reinterpret_cast<struct multiboot_tag_mmap *>(tag))
                                ->entries;
                     reinterpret_cast<multiboot_uint8_t *>(mmap) <
                     reinterpret_cast<multiboot_uint8_t *>(tag) + tag->size;
                     mmap = reinterpret_cast<multiboot_memory_map_t *>(
                         reinterpret_cast<addr_t>(mmap) +
                         (reinterpret_cast<struct multiboot_tag_mmap *>(tag))
                             ->entry_size)) {
                    log::printk(log::log_level::INFO,
                                "    [%p - %p] Type: 0x%X\n",
                                static_cast<addr_t>(mmap->addr),
                                static_cast<addr_t>(mmap->addr) +
                                    static_cast<addr_t>(mmap->len),
                                static_cast<addr_t>(mmap->type));
                    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                        for (addr_t i = mmap->addr; i < mmap->addr + mmap->len;
                             i += memory::virt::PAGE_SIZE) {
                            if (memory::x86::is_valid_physical_memory(i,
                                                                      info)) {
                                memory::physical::free(i);
                            }
                        }
                    }
                }
            } break;
        }
    }

    // Preload the top level page table entries
#ifdef X86_64
    struct memory::virt::page_table *top_level =
        (struct memory::virt::page_table *)memory::x86::decode_fractal(
            memory::x86::recursive_entry, memory::x86::recursive_entry,
            memory::x86::recursive_entry, memory::x86::recursive_entry);
#else
    struct memory::virt::page_table *top_level =
        (struct memory::virt::page_table *)memory::x86::decode_fractal(
            memory::x86::recursive_entry, memory::x86::recursive_entry);

#endif
#ifdef X86_64
    for (int i = 256; i < 512; i++) {
#else
    for (int i = 768; i < 1024; i++) {
#endif
        if (!top_level->pages[i].present) {
#ifdef X86_64
            struct memory::virt::page_table *second_level =
                (struct memory::virt::page_table *)memory::x86::decode_fractal(
                    memory::x86::recursive_entry, memory::x86::recursive_entry,
                    memory::x86::recursive_entry, i);
#else
            struct memory::virt::page_table *second_level =
                (struct memory::virt::page_table *)memory::x86::decode_fractal(
                    memory::x86::recursive_entry, i);
#endif
            top_level->pages[i].present  = 1;
            top_level->pages[i].writable = 1;
            top_level->pages[i].address = memory::physical::allocate() / 0x1000;
            memory::x86::invlpg((addr_t)second_level);
            libcxx::memset(second_level, 0, sizeof(*second_level));
        }
    }
} // namespace memory
} // namespace memory
