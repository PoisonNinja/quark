#include <kernel.h>
#include <kernel/module.h>
#include <kernel/symbol.h>
#include <lib/string.h>
#include <proc/elf.h>

namespace
{
struct Module {
    ~Module()
    {
        delete[] shdrs;
        delete[] sections;
    }

    const char* name;

    size_t shnum;

    // Dynamically allocated
    ELF::Elf_Shdr* shdrs;
    addr_t* sections;

    // Module entry points
    int (*init)();
    int (*fini)();

    Node<Module> node;
};

List<Module, &Module::node> modules;
}  // namespace

bool load_module(void* binary)
{
    Log::printk(Log::LogLevel::INFO, "[load_module] Loading module at %p\n",
                binary);
    ELF::Elf_Ehdr* header = reinterpret_cast<ELF::Elf_Ehdr*>(binary);
    if (String::memcmp(header->e_ident, ELFMAG, 4)) {
        Log::printk(Log::LogLevel::ERROR,
                    "[load_module] Binary passed in is not an ELF file!\n");
        return false;
    }

    struct Module* mod = new Module;

    mod->shnum = header->e_shnum;
    mod->shdrs = new ELF::Elf_Shdr[mod->shnum];
    mod->sections = new addr_t[mod->shnum];

    for (uint32_t i = 0; i < header->e_shnum; i++) {
        ELF::Elf_Shdr* shdr =
            reinterpret_cast<ELF::Elf_Shdr*>(
                (reinterpret_cast<addr_t>(binary) + header->e_shoff)) +
            i;
        String::memcpy(&mod->shdrs[i], shdr, header->e_shentsize);
    }

    for (uint32_t i = 0; i < header->e_shnum; i++) {
        mod->shdrs[i].sh_addr = mod->sections[i] =
            reinterpret_cast<ELF::Elf_Addr>(new uint8_t[mod->shdrs[i].sh_size]);
        if (mod->shdrs[i].sh_type == SHT_NOBITS) {
            String::memset(reinterpret_cast<void*>(mod->sections[i]), 0,
                           mod->shdrs[i].sh_size);
        } else {
            String::memcpy(
                reinterpret_cast<void*>(mod->sections[i]),
                reinterpret_cast<void*>((reinterpret_cast<addr_t>(header) +
                                         mod->shdrs[i].sh_offset)),
                mod->shdrs[i].sh_size);
        }
    }

    ELF::Elf_Shdr* shdr_string_table = &mod->shdrs[header->e_shstrndx];
    Log::printk(Log::LogLevel::DEBUG,
                "[load_module] shdr_string_table: %p, type: %X, offset: %p\n",
                shdr_string_table, shdr_string_table->sh_type,
                shdr_string_table->sh_offset);
    const char* s_string_table =
        reinterpret_cast<const char*>((mod->sections[header->e_shstrndx]));

    const char* string_table = nullptr;
    for (uint32_t i = 0; i < header->e_shnum; i++) {
        if (mod->shdrs[i].sh_type == SHT_STRTAB &&
            !String::strcmp(".strtab",
                            s_string_table + mod->shdrs[i].sh_name)) {
            string_table = reinterpret_cast<const char*>(mod->sections[i]);
        }
    }

    Log::printk(Log::LogLevel::DEBUG, "[load_module] String table at %p\n",
                string_table);

    ELF::Elf_Sym* symtab;
    size_t num_syms;
    for (uint32_t i = 0; i < header->e_shnum; i++) {
        if (mod->shdrs[i].sh_type == SHT_SYMTAB) {
            symtab = reinterpret_cast<ELF::Elf_Sym*>(mod->sections[i]);
            num_syms = mod->shdrs[i].sh_size / mod->shdrs[i].sh_entsize;
        }
    }

    for (uint32_t i = 0; i < header->e_shnum; i++) {
        if (mod->shdrs[i].sh_type == SHT_RELA) {
            for (uint32_t x = 0; x < mod->shdrs[i].sh_size;
                 x += mod->shdrs[i].sh_entsize) {
                ELF::Elf_Rela* rel =
                    reinterpret_cast<ELF::Elf_Rela*>(mod->sections[i] + x);
                ELF::Elf_Sym* sym = symtab + ELF_R_SYM(rel->r_info);
                Log::printk(Log::LogLevel::DEBUG, "[load_module] Name: %s\n",
                            string_table + sym->st_name);
                addr_t symaddr = 0;
                if (sym->st_shndx == 0) {
                    addr_t temp =
                        Symbols::resolve_name(string_table + sym->st_name);
                    if (!temp) {
                        Log::printk(Log::LogLevel::ERROR,
                                    "[load_module] Undefined reference to %s\n",
                                    string_table + sym->st_name);
                        return false;
                    }
                    symaddr = temp;
                } else if (sym->st_shndx) {
                    symaddr = mod->sections[sym->st_shndx] + sym->st_value;
                }
                addr_t target = mod->sections[mod->shdrs[i].sh_info];
                target += rel->r_offset;
                switch (ELF_R_TYPE(rel->r_info)) {
                    case R_X86_64_64:
                        Log::printk(Log::LogLevel::DEBUG,
                                    "[load_module] R_X86_64_64: %p %p %p %X\n",
                                    rel->r_addend, rel->r_offset, symaddr,
                                    mod->shdrs[i].sh_info);
                        *(reinterpret_cast<addr_t*>(target)) =
                            symaddr + rel->r_addend;
                        break;
                    default:
                        Log::printk(
                            Log::LogLevel::ERROR,
                            "[load_module] Unsupported relocation type: %d\n",
                            ELF_R_TYPE(rel->r_info));
                        break;
                }
            }
        }
    }
    ELF::Elf_Sym* sym = symtab;
    for (size_t j = 1; j <= num_syms; j++, sym++) {
        // We only want to consider functions
        if (ELF_ST_TYPE(sym->st_info) != STT_FUNC)
            continue;
        if (!String::strcmp(string_table + sym->st_name, "init")) {
            Log::printk(Log::LogLevel::DEBUG,
                        "[load_module] Located init point at %p\n",
                        mod->sections[sym->st_shndx] + sym->st_value);
            mod->init = reinterpret_cast<int (*)()>(
                mod->sections[sym->st_shndx] + sym->st_value);
        } else if (!String::strcmp(string_table + sym->st_name, "fini")) {
            Log::printk(Log::LogLevel::DEBUG,
                        "[load_module] Located init point at %p\n",
                        mod->sections[sym->st_shndx] + sym->st_value);
            mod->fini = reinterpret_cast<int (*)()>(
                mod->sections[sym->st_shndx] + sym->st_value);
        }
    }

    modules.push_front(*mod);
    mod->init();
    return true;
}