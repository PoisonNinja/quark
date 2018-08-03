#include <kernel.h>
#include <kernel/module.h>
#include <kernel/symbol.h>
#include <lib/string.h>
#include <proc/elf.h>

namespace
{
struct Module {
    Module()
    {
        shdrs = nullptr;
        sections = nullptr;
        init = nullptr;
        fini = nullptr;
        name = description = version = author = nullptr;
    }
    ~Module()
    {
        delete[] shdrs;
        delete[] sections;
    }

    const char *name, *description, *version, *author;

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

// A ghetto version of strtok that accepts \0
const char* next_token(const char* string, const char* end)
{
    const char* p = string;
    while (*p != '\0')
        p++;
    while (*p == '\0') {
        if (p + 1 == end)
            return nullptr;
        p++;
    }
    return p;
}

void parse_modinfo(Module* mod, size_t index)
{
    const char* key = (char*)(mod->sections[index]);
    const char* end = (char*)(mod->sections[index] + mod->shdrs[index].sh_size);

    int found = 0;
    while (key) {
        Log::printk(Log::LogLevel::DEBUG, "[parse_modinfo]: Key: %s\n", key);

        if (!String::strncmp("name=", key, 5)) {
            mod->name = key + 5;
            found++;
        } else if (!String::strncmp("description=", key, 12)) {
            mod->description = key + 12;
            found++;
        } else if (!String::strncmp("version=", key, 8)) {
            mod->version = key + 8;
            found++;
        } else if (!String::strncmp("author=", key, 7)) {
            mod->author = key + 7;
            found++;
        } else {
            Log::printk(Log::LogLevel::WARNING,
                        "[parse_modinfo]: Invalid token %s\n", key);
        }

        key = next_token(key, end);
    }

    if (found != 4) {
        Log::printk(Log::LogLevel::WARNING,
                    "[parse_modinfo]: Missing required attributes\n");
    }
}
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
    size_t modinfo_index = 0;
    for (uint32_t i = 0; i < header->e_shnum; i++) {
        if (mod->shdrs[i].sh_type == SHT_STRTAB &&
            !String::strcmp(".strtab",
                            s_string_table + mod->shdrs[i].sh_name)) {
            string_table = reinterpret_cast<const char*>(mod->sections[i]);
        } else if (mod->shdrs[i].sh_type == SHT_PROGBITS &&
                   !String::strcmp(".modinfo",
                                   s_string_table + mod->shdrs[i].sh_name)) {
            modinfo_index = i;
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
            break;
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

    // The strings in modinfo are relocated so we need to wait until we
    // completed relocations
    parse_modinfo(mod, modinfo_index);

    if (!mod->name) {
        Log::printk(Log::LogLevel::ERROR,
                    "[load_module]: Missing module name, not loading\n");
        return false;
        delete mod;
    }

    Log::printk(Log::LogLevel::INFO, "[load_module] Loaded module %s\n",
                mod->name);
    Log::printk(Log::LogLevel::INFO, "[load_module] Description: %s\n",
                (mod->description) ? mod->description : "");
    Log::printk(Log::LogLevel::INFO, "[load_module] Author: %s\n",
                (mod->author) ? mod->author : "");
    Log::printk(Log::LogLevel::INFO, "[load_module] Version: %s\n",
                (mod->version) ? mod->version : "");

    modules.push_front(*mod);
    mod->init();

    return true;
}