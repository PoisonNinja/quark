#include <arch/cpu/feature.h>
#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <arch/mm/mm.h>
#include <arch/mm/virtual.h>
#include <boot/info.h>
#include <cpu/cpu.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace memory
{
namespace
{
size_t max_valid_size(addr_t address, struct boot::info &info,
                      addr_t mboot_start, addr_t mboot_end, addr_t zone_end)
{
    int num_zeros = __builtin_ctzll(address);
    // TODO: Use MAX_ORDER instead of magic number
    if (num_zeros > 28)
        num_zeros = 28;
    size_t buddy_size;
    for (buddy_size = (1 << num_zeros); buddy_size > memory::virt::PAGE_SIZE;
         buddy_size >>= 1) {
        addr_t end = address + buddy_size;
        if (end > zone_end)
            continue;
        else if (address <= info.kernel_start && end >= info.kernel_end)
            continue;
        else if (address <= info.initrd_start && end >= info.initrd_end)
            continue;
        else if (address <= mboot_start && end >= mboot_end)
            continue;
        else if (!memory::x86_64::is_valid_physical_memory(
                     end, info, mboot_start, mboot_end))
            continue;
        break;
    }
    return buddy_size;
}
} // namespace

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
                        for (addr_t i = mmap->addr;
                             i < memory::virt::align_down(mmap->addr +
                                                          mmap->len);) {
                            if (memory::x86_64::is_valid_physical_memory(
                                    i, info, multiboot_start, multiboot_end)) {
                                size_t size = max_valid_size(
                                    i, info, multiboot_start, multiboot_end,
                                    memory::virt::align_down(mmap->addr +
                                                             mmap->len));
                                memory::physical::free(i, size);
                                i += size;
                            } else {
                                i += memory::virt::PAGE_SIZE;
                            }
                        }
                    }
                }
            } break;
        }
    }
}

void arch_post_init(struct boot::info &info)
{
    // Preload the top level page table entries to synchronize them between
    // threads
    struct memory::virt::page_table *top_level =
        (struct memory::virt::page_table *)memory::x86_64::decode_fractal(
            memory::x86_64::recursive_entry, memory::x86_64::recursive_entry,
            memory::x86_64::recursive_entry, memory::x86_64::recursive_entry);
    for (int i = 256; i < 512; i++) {
        if (!top_level->pages[i].present) {
            struct memory::virt::page_table *second_level =
                static_cast<struct memory::virt::page_table *>(
                    memory::x86_64::decode_fractal(
                        memory::x86_64::recursive_entry,
                        memory::x86_64::recursive_entry,
                        memory::x86_64::recursive_entry, i));
            top_level->pages[i].present  = 1;
            top_level->pages[i].writable = 1;
            top_level->pages[i].address = memory::physical::allocate() / 0x1000;
            memory::x86_64::invlpg(reinterpret_cast<addr_t>(second_level));
            libcxx::memset(second_level, 0, sizeof(*second_level));
        }
    }
}
} // namespace memory
