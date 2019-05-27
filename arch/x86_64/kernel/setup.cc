#include <arch/cpu/cpu.h>
#include <arch/kernel/multiboot2.h>
#include <boot/info.h>
#include <cpu/interrupt.h>
#include <drivers/tty/serial.h>
#include <kernel.h>
#include <kernel/symbol.h>
#include <lib/libcxx.h>
#include <lib/string.h>

extern void kmain(struct boot::info &info);

namespace symbols
{
void relocate(struct multiboot_tag_elf_sections *sections);
} // namespace symbols

namespace memory
{
namespace physical
{
void init_early_alloc(struct boot::info *m);
addr_t early_allocate();
} // namespace physical
} // namespace memory

namespace x86_64
{
extern "C" {
void *__constructors_start;
void *__constructors_end;

void *__kernel_start;
void *__kernel_end;
}

namespace
{
serial serial_console;
struct boot::info info;
} // namespace

void init(uint32_t magic, struct multiboot_fixed *multiboot)
{
    // Get a serial console running so we can output stuff
    log::register_log_output(serial_console);
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        kernel::panic("Booted by a non-supported bootloader!\n");
    }

    // Start filling out information struct
    info.architecture_data = multiboot;
    info.kernel_start      = reinterpret_cast<addr_t>(&__kernel_start);
    info.kernel_end        = reinterpret_cast<addr_t>(&__kernel_end);

    struct multiboot_tag_elf_sections *sections = nullptr;
    struct multiboot_tag *tag;
    for (tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<addr_t>(multiboot) + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<multiboot_uint8_t *>(tag) +
             ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                info.cmdline =
                    reinterpret_cast<struct multiboot_tag_string *>(tag)
                        ->string;
                break;
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                log::printk(
                    log::log_level::INFO, "Booted by %s\n",
                    (reinterpret_cast<struct multiboot_tag_string *>(tag))
                        ->string);
                break;
            case MULTIBOOT_TAG_TYPE_MODULE:
                info.initrd_start =
                    (reinterpret_cast<struct multiboot_tag_module *>(tag))
                        ->mod_start;
                info.initrd_end =
                    (reinterpret_cast<struct multiboot_tag_module *>(tag))
                        ->mod_end;
                break;
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
                log::printk(log::log_level::INFO, "Located symbol section\n");
                sections = (struct multiboot_tag_elf_sections *)tag;
                break;
        }
    }
    // Bootstrap the IDT and GDT
    cpu::x86_64::init();

    /*
     * Initialize our physical memory early allocator so we can start using
     * memory immediately.
     */
    memory::physical::init_early_alloc(&info);

    /*
     * Load the symbols now. We used to do this much later, but the symbols
     * might get overwritten later once we start using more memory.
     */
    symbols::relocate(sections);
    kmain(info);
}

extern "C" {
void asm_to_cxx_trampoline(uint32_t magic, struct multiboot_fixed *multiboot)
{
    libcxx::constructors_initialize(&__constructors_start, &__constructors_end);
    x86_64::init(magic, multiboot);
}
}
} // namespace x86_64
