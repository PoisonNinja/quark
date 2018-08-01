#include <arch/cpu/cpu.h>
#include <arch/kernel/multiboot2.h>
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

#include <proc/elf.h>

// static void multiboot_parse_symbols(struct multiboot_tag_elf_sections *table)
// {
//     uint64_t *sections = (uint64_t *)table->sections;

//     // Locate the section header string table
//     ELF::Elf_Shdr *shdr_string_table = (ELF::Elf_Shdr *)sections +
//     table->shndx; char *s_string_table =
//         (char *)(shdr_string_table->sh_addr + 0xFFFFFFFF80000000);

//     ELF::Elf_Shdr *string_table_header = nullptr;
//     char *string_table = nullptr;
//     // ELF binaries generally have three or four string tables.
//     // Locate the correct string table (.symtab)
//     for (uint32_t i = 0; i < table->num; i++) {
//         ELF::Elf_Shdr *shdr = (ELF::Elf_Shdr *)sections + i;
//         if (shdr->sh_type == SHT_STRTAB &&
//             !String::strcmp(".strtab", s_string_table + shdr->sh_name)) {
//             string_table_header = (ELF::Elf_Shdr *)sections + i;
//             string_table =
//                 (char *)(string_table_header->sh_addr + 0xFFFFFFFF80000000);
//         }
//     }

//     if (!string_table) {
//         Log::printk(Log::LogLevel::WARNING, "Failed to locate string
//         table\n"); return;
//     }

//     // Locate the symbol table
//     for (uint32_t i = 0; i < table->num; i++) {
//         ELF::Elf_Shdr *shdr = (ELF::Elf_Shdr *)sections + i;
//         if (shdr->sh_type == SHT_SYMTAB) {
//             ELF::Elf_Sym *symtab =
//                 (ELF::Elf_Sym *)(shdr->sh_addr + 0xFFFFFFFF80000000);
//             if (!symtab)
//                 continue;
//             int num_syms = shdr->sh_size / shdr->sh_entsize;
//             for (int j = 1; j <= num_syms; j++) {
//                 symtab++;
//                 if (ELF64_ST_TYPE(symtab->st_info) != STT_FUNC)
//                     continue;
//                 const char *name;
//                 if (symtab->st_name != 0) {
//                     name = string_table + symtab->st_name;
//                 } else {
//                     name = "N/A";
//                 }
//                 Log::printk(Log::LogLevel::DEBUG, "%s: %p\n", name,
//                             symtab->st_value);
//             }
//         } else if (shdr->sh_type == SHT_STRTAB) {
//             Log::printk(Log::LogLevel::INFO, "Found string table at %u\n",
//             i);
//         }
//     }
// }

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
