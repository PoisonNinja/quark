#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/symbol.h>
#include <lib/pair.h>
#include <lib/string.h>
#include <proc/elf.h>

namespace Symbols
{
void init(struct multiboot_tag_elf_sections *tag)
{
    addr_t *sections = (addr_t *)tag->sections;

    // Locate the section header string table
    ELF::Elf_Shdr *shdr_string_table = (ELF::Elf_Shdr *)sections + tag->shndx;
    char *s_string_table = (char *)(shdr_string_table->sh_addr + VMA);

    // ELF binaries generally have three or four string tables.
    // Locate the correct string table (.symtab)
    ELF::Elf_Shdr *string_table_header = nullptr;
    char *string_table = nullptr;
    for (uint32_t i = 0; i < tag->num; i++) {
        ELF::Elf_Shdr *shdr = (ELF::Elf_Shdr *)sections + i;
        if (shdr->sh_type == SHT_STRTAB &&
            !String::strcmp(".strtab", s_string_table + shdr->sh_name)) {
            string_table_header = (ELF::Elf_Shdr *)sections + i;
            string_table = (char *)(string_table_header->sh_addr + VMA);
        }
    }

    if (!string_table) {
        Log::printk(Log::LogLevel::WARNING, "Failed to locate string table\n");
        return;
    }

    ELF::Elf_Sym *symtab = nullptr;
    int num_syms = 0;
    for (uint32_t i = 0; i < tag->num; i++) {
        ELF::Elf_Shdr *shdr = (ELF::Elf_Shdr *)sections + i;
        if (shdr->sh_type == SHT_SYMTAB) {
            symtab = (ELF::Elf_Sym *)(shdr->sh_addr + VMA);
            num_syms = shdr->sh_size / shdr->sh_entsize;
        }
    }

    ELF::Elf_Sym *sym = symtab;
    for (int j = 1; j <= num_syms; j++, sym++) {
        if (ELF64_ST_TYPE(sym->st_info) != STT_FUNC &&
            ELF64_ST_TYPE(sym->st_info) != STT_OBJECT)
            continue;
        Symbols::load_symbol(Pair<const char *, addr_t>(
            string_table + sym->st_name, sym->st_value));
    }
    return;
}  // namespace Symbols
}  // namespace Symbols
