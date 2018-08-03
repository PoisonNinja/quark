#include <arch/common/kernel/multiboot2.h>
#include <arch/cpu/cpu.h>
#include <boot/info.h>
#include <cpu/interrupt.h>
#include <drivers/tty/serial.h>
#include <kernel.h>
#include <lib/libcxx.h>
#include <lib/string.h>

extern void kmain(struct Boot::info &info);

namespace Symbols
{
void set_table(struct multiboot_tag_elf_sections *t);
}

namespace X64
{
extern "C" {
void *__constructors_start;
void *__constructors_end;

void *__kernel_start;
void *__kernel_end;
}

static Serial serial_console;
static struct Boot::info info;

void init(uint32_t magic, struct multiboot_fixed *multiboot)
{
    Log::register_log_output(serial_console);
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        Log::printk(Log::LogLevel::ERROR,
                    "Multiboot magic number does not match!\n");
    }
    info.architecture_data = multiboot;
    info.kernel_start = reinterpret_cast<addr_t>(&__kernel_start);
    info.kernel_end = reinterpret_cast<addr_t>(&__kernel_end);
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
                Log::printk(
                    Log::LogLevel::INFO, "Booted by %s\n",
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
                Log::printk(Log::LogLevel::INFO, "Located symbol section\n");
                struct multiboot_tag_elf_sections *sections =
                    (struct multiboot_tag_elf_sections *)tag;
                Symbols::set_table(sections);
                break;
        }
    }
    CPU::X64::init();
    kmain(info);
}

extern "C" {
void asm_to_cxx_trampoline(uint32_t magic, struct multiboot_fixed *multiboot)
{
    libcxx::constructors_initialize(&__constructors_start, &__constructors_end);
    X64::init(magic, multiboot);
}
}
}  // namespace X64
