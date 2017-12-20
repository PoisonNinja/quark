#include <arch/kernel/multiboot2.h>
#include <arch/mm/mm.h>
#include <boot/info.h>
#include <kernel.h>

#include <mm/virtual.h>

static __attribute__((aligned(0x1000))) char sentinel[0x1000];

namespace Memory
{
void arch_init(struct Boot::info &info)
{
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
                             ->entry_size))
                    Log::printk(Log::INFO,
                                "    Base = 0x%016x, Length = 0x%016x, "
                                "Type = 0x%x\n",
                                static_cast<addr_t>(mmap->addr),
                                static_cast<addr_t>(mmap->len),
                                static_cast<addr_t>(mmap->type));
            } break;
        }
    }
    Memory::Virtual::map(reinterpret_cast<addr_t>(&sentinel), 0xBEEF0000,
                         PAGE_PRESENT);
}
}  // namespace Memory
