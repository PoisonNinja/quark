#include <arch/common/kernel/multiboot2.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/symbol.h>
#include <lib/pair.h>
#include <lib/string.h>
#include <proc/elf.h>

namespace
{
struct multiboot_tag_elf_sections *table = nullptr;
ELF::Elf_Shdr *string_table_header = nullptr;
char *string_table = nullptr;
ELF::Elf_Sym *symtab = nullptr;
int num_syms = 0;

int init()
{
    uint32_t *sections = (uint32_t *)table->sections;

    // Locate the section header string table
    ELF::Elf_Shdr *shdr_string_table = (ELF::Elf_Shdr *)sections + table->shndx;
    char *s_string_table = (char *)(shdr_string_table->sh_addr + 0xC0000000);

    // ELF binaries generally have three or four string tables.
    // Locate the correct string table (.symtab)
    if (!string_table) {
        for (uint32_t i = 0; i < table->num; i++) {
            ELF::Elf_Shdr *shdr = (ELF::Elf_Shdr *)sections + i;
            if (shdr->sh_type == SHT_STRTAB &&
                !String::strcmp(".strtab", s_string_table + shdr->sh_name)) {
                string_table_header = (ELF::Elf_Shdr *)sections + i;
                string_table =
                    (char *)(string_table_header->sh_addr + 0xC0000000);
            }
        }

        if (!string_table) {
            Log::printk(Log::LogLevel::WARNING,
                        "Failed to locate string table\n");
            return -1;
        }
    }

    // Locate the symbol table
    if (!symtab) {
        for (uint32_t i = 0; i < table->num; i++) {
            ELF::Elf_Shdr *shdr = (ELF::Elf_Shdr *)sections + i;
            if (shdr->sh_type == SHT_SYMTAB) {
                symtab = (ELF::Elf_Sym *)(shdr->sh_addr + 0xC0000000);
                num_syms = shdr->sh_size / shdr->sh_entsize;
            }
        }
    }

    ELF::Elf_Sym *sym = symtab;
    for (int j = 1; j <= num_syms; j++, sym++) {
        if (ELF_ST_TYPE(sym->st_info) != STT_FUNC &&
            ELF_ST_TYPE(sym->st_info) != STT_OBJECT)
            continue;
        Symbols::load_symbol(Pair<const char *, addr_t>(
            string_table + sym->st_name, sym->st_value));
    }
    return 0;
}
EARLY_INITCALL(init);

}  // namespace

namespace Symbols
{
void set_table(struct multiboot_tag_elf_sections *t)
{
    table = t;
}

Pair<const char *, size_t> primitive_resolve_addr(addr_t address)
{
    size_t difference = ~0;
    const char *ret = nullptr;

    uint32_t *sections = (uint32_t *)table->sections;

    // Locate the section header string table
    ELF::Elf_Shdr *shdr_string_table = (ELF::Elf_Shdr *)sections + table->shndx;
    char *s_string_table = (char *)(shdr_string_table->sh_addr + 0xC0000000);

    // ELF binaries generally have three or four string tables.
    // Locate the correct string table (.symtab)
    if (!string_table) {
        for (uint32_t i = 0; i < table->num; i++) {
            ELF::Elf_Shdr *shdr = (ELF::Elf_Shdr *)sections + i;
            if (shdr->sh_type == SHT_STRTAB &&
                !String::strcmp(".strtab", s_string_table + shdr->sh_name)) {
                string_table_header = (ELF::Elf_Shdr *)sections + i;
                string_table =
                    (char *)(string_table_header->sh_addr + 0xC0000000);
            }
        }

        if (!string_table) {
            Log::printk(Log::LogLevel::WARNING,
                        "Failed to locate string table\n");
            return Pair<const char *, size_t>(nullptr, 0);
        }
    }

    // Locate the symbol table
    if (!symtab) {
        for (uint32_t i = 0; i < table->num; i++) {
            ELF::Elf_Shdr *shdr = (ELF::Elf_Shdr *)sections + i;
            if (shdr->sh_type == SHT_SYMTAB) {
                symtab = (ELF::Elf_Sym *)(shdr->sh_addr + 0xC0000000);
                num_syms = shdr->sh_size / shdr->sh_entsize;
            }
        }
    }

    ELF::Elf_Sym *sym = symtab;
    for (int j = 1; j <= num_syms; j++) {
        sym++;
        if (ELF_ST_TYPE(sym->st_info) != STT_FUNC &&
            ELF_ST_TYPE(sym->st_info) != STT_OBJECT)
            continue;
        if (sym->st_value <= address) {
            size_t temp = address - sym->st_value;
            if (temp < difference) {
                ret = string_table + sym->st_name;
                difference = temp;
            }
        }
    }
    return make_pair(ret, difference);
}
}  // namespace Symbols