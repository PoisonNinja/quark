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
void init_phys_map()
{
    bool supports_1gb = false;
    if (cpu::x86_64::has_feature(*cpu::get_current_core(),
                                 X86_FEATURE_GBPAGES)) {
        supports_1gb = true;
        log::printk(log::log_level::INFO,
                    "CPU supports 1 GB pages, using it\n");
    } else {
        log::printk(
            log::log_level::WARNING,
            "CPU does not support 1 GB pages, falling back to 2MB pages\n");
    }
    struct memory::virt::page_table *top_level =
        (struct memory::virt::page_table *)memory::x86_64::decode_fractal(
            memory::x86_64::recursive_entry, memory::x86_64::recursive_entry,
            memory::x86_64::recursive_entry, memory::x86_64::recursive_entry);
    struct memory::virt::page_table *second_level =
        static_cast<struct memory::virt::page_table *>(
            memory::x86_64::decode_fractal(
                memory::x86_64::recursive_entry,
                memory::x86_64::recursive_entry,
                memory::x86_64::recursive_entry,
                memory::x86_64::pml4_index(PHYS_START)));
    top_level->pages[memory::x86_64::pml4_index(PHYS_START)].present  = 1;
    top_level->pages[memory::x86_64::pml4_index(PHYS_START)].writable = 1;
    top_level->pages[memory::x86_64::pml4_index(PHYS_START)].address =
        memory::physical::allocate() / 0x1000;
    memory::x86_64::invlpg(reinterpret_cast<addr_t>(second_level));
    libcxx::memset(second_level, 0, sizeof(*second_level));
    for (int i = 0; i < 512; i++) {
        second_level->pages[i].present  = 1;
        second_level->pages[i].nx       = 1;
        second_level->pages[i].writable = 1;
        if (supports_1gb) {
            second_level->pages[i].huge_page = 1;
            second_level->pages[i].address   = i * 0x40000;
        } else {
            /*
             * TODO: Tune this based on how much actual physical memory is
             * present. It's probably overkill to map 256 TB if only 128 MB
             * is present
             */
            struct memory::virt::page_table *third_level =
                static_cast<struct memory::virt::page_table *>(
                    memory::x86_64::decode_fractal(
                        memory::x86_64::recursive_entry,
                        memory::x86_64::recursive_entry,
                        memory::x86_64::pml4_index(PHYS_START), i));
            second_level->pages[i].address =
                memory::physical::allocate() / 0x1000;
            memory::x86_64::invlpg(reinterpret_cast<addr_t>(second_level));
            libcxx::memset(third_level, 0, sizeof(*third_level));
            for (int j = 0; j < 512; j++) {
                third_level->pages[j].present   = 1;
                third_level->pages[j].nx        = 1;
                third_level->pages[j].writable  = 1;
                third_level->pages[j].huge_page = 1;
                third_level->pages[j].address   = (0x40000 * i) + (0x200 * j);
            }
        }
    }
}
} // namespace

void arch_init(struct boot::info &info)
{
    init_phys_map();
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
                             i <
                             memory::virt::align_down(mmap->addr + mmap->len);
                             i += memory::virt::PAGE_SIZE) {
                            if (memory::x86_64::is_valid_physical_memory(
                                    i, info)) {
                                memory::physical::free(i);
                            }
                        }
                    }
                }
            } break;
        }
    }

    // Preload the top level page table entries
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
} // namespace memory
} // namespace memory
