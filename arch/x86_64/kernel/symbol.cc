#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/symbol.h>
#include <lib/string.h>
#include <lib/utility.h>
#include <proc/binfmt/elf.h>

namespace
{
char *string_table   = nullptr;
elf::elf_sym *symtab = nullptr;
int num_syms         = 0;
} // namespace

namespace symbols
{
void relocate(struct multiboot_tag_elf_sections *tag)
{
    addr_t *sections = reinterpret_cast<addr_t *>(tag->sections);

    // Locate the section header string table
    elf::elf_shdr *shdr_string_table =
        reinterpret_cast<elf::elf_shdr *>(sections) + tag->shndx;
    char *s_string_table =
        reinterpret_cast<char *>(shdr_string_table->sh_addr + VMA);

    // elf binaries generally have three or four string tables.
    // Locate the correct string table (.symtab)
    elf::elf_shdr *string_table_header = nullptr;
    char *temp_string_table            = nullptr;
    for (uint32_t i = 0; i < tag->num; i++) {
        elf::elf_shdr *shdr = (elf::elf_shdr *)sections + i;
        if (shdr->sh_type == SHT_STRTAB &&
            !libcxx::strcmp(".strtab", s_string_table + shdr->sh_name)) {
            string_table_header =
                reinterpret_cast<elf::elf_shdr *>(sections) + i;
            temp_string_table =
                reinterpret_cast<char *>(string_table_header->sh_addr + VMA);
            string_table = new char[string_table_header->sh_size];
            libcxx::memcpy(string_table, temp_string_table,
                           string_table_header->sh_size);
        }
    }

    if (!temp_string_table) {
        log::printk(log::log_level::WARNING, "Failed to locate string table\n");
        return;
    }

    // Locate the symbol table
    elf::elf_sym *temp_symtab;
    for (uint32_t i = 0; i < tag->num; i++) {
        elf::elf_shdr *shdr = reinterpret_cast<elf::elf_shdr *>(sections) + i;
        if (shdr->sh_type == SHT_SYMTAB) {
            temp_symtab = reinterpret_cast<elf::elf_sym *>(shdr->sh_addr + VMA);
            num_syms    = shdr->sh_size / shdr->sh_entsize;
            symtab      = new elf::elf_sym[shdr->sh_size];
            libcxx::memcpy(symtab, temp_symtab, shdr->sh_size);
        }
    }
}

void arch_init()
{
    elf::elf_sym *sym = symtab;
    for (int j = 0; j < num_syms; j++, sym++) {
        /*
         * We currently only care for global objects and functions. In reality,
         * these are probably the only items anyways but we'll occasionally get
         * some other stuff
         */
        if (ELF64_ST_TYPE(sym->st_info) != STT_FUNC &&
            ELF64_ST_TYPE(sym->st_info) != STT_OBJECT)
            continue;
        symbols::load_symbol(libcxx::pair<const char *, addr_t>(
            string_table + sym->st_name, sym->st_value));
    }
    return;
} // namespace symbols
} // namespace symbols
