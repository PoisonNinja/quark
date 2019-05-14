#include <kernel.h>
#include <kernel/module.h>
#include <kernel/symbol.h>
#include <lib/string.h>
#include <proc/elf.h>

namespace
{
libcxx::list<module, &module::node> modules;

// A ghetto version of strtok that accepts \0
const char* next_token(const char* string, const char* end)
{
    const char* p = string;
    // Skip until the end of the current key
    while (*p != '\0')
        p++;
    /*
     * Skip until the start of the next key. Due to the way that we store keys,
     * there will be empty space in between keys for padding which are made of
     * null terminators
     */
    while (*p == '\0') {
        // Check if we are overrunning the end of the entire key section
        if (p + 1 == end)
            return nullptr;
        p++;
    }
    // By now p will be pointing to the start of the next key
    return p;
}

void parse_modinfo(module* mod, size_t index)
{
    const char* key = (char*)(mod->sections[index]);
    const char* end = (char*)(mod->sections[index] + mod->shdrs[index].sh_size);

    int found = 0;
    while (key) {
        log::printk(log::log_level::DEBUG, "[parse_modinfo]: Key: %s\n", key);

        if (!libcxx::strncmp("name=", key, 5)) {
            mod->name = key + 5;
            found++;
        } else if (!libcxx::strncmp("description=", key, 12)) {
            mod->description = key + 12;
            found++;
        } else if (!libcxx::strncmp("version=", key, 8)) {
            mod->version = key + 8;
            found++;
        } else if (!libcxx::strncmp("author=", key, 7)) {
            mod->author = key + 7;
            found++;
        } else {
            log::printk(log::log_level::WARNING,
                        "[parse_modinfo]: Invalid token %s\n", key);
        }

        key = next_token(key, end);
    }

    if (found != 4) {
        log::printk(log::log_level::WARNING,
                    "[parse_modinfo]: Missing required attributes\n");
    }
}
} // namespace

extern bool relocate_module(module* mod, elf::elf_sym* symtab,
                            const char* string_table);

bool load_module(void* binary)
{
    log::printk(log::log_level::INFO, "[load_module] Loading module at %p\n",
                binary);
    elf::elf_ehdr* header = reinterpret_cast<elf::elf_ehdr*>(binary);
    if (libcxx::memcmp(header->e_ident, ELFMAG, 4)) {
        log::printk(log::log_level::ERROR,
                    "[load_module] Binary passed in is not an elf file!\n");
        return false;
    }

    struct module* mod = new module;

    mod->shnum    = header->e_shnum;
    mod->shdrs    = new elf::elf_shdr[mod->shnum];
    mod->sections = new addr_t[mod->shnum];

    for (uint32_t i = 0; i < header->e_shnum; i++) {
        elf::elf_shdr* shdr =
            reinterpret_cast<elf::elf_shdr*>(
                (reinterpret_cast<addr_t>(binary) + header->e_shoff)) +
            i;
        libcxx::memcpy(&mod->shdrs[i], shdr, header->e_shentsize);
    }

    for (uint32_t i = 0; i < header->e_shnum; i++) {
        mod->shdrs[i].sh_addr = mod->sections[i] =
            reinterpret_cast<elf::elf_addr>(new uint8_t[mod->shdrs[i].sh_size]);
        if (mod->shdrs[i].sh_type == SHT_NOBITS) {
            libcxx::memset(reinterpret_cast<void*>(mod->sections[i]), 0,
                           mod->shdrs[i].sh_size);
        } else {
            libcxx::memcpy(
                reinterpret_cast<void*>(mod->sections[i]),
                reinterpret_cast<void*>((reinterpret_cast<addr_t>(header) +
                                         mod->shdrs[i].sh_offset)),
                mod->shdrs[i].sh_size);
        }
    }

    elf::elf_shdr* shdr_string_table = &mod->shdrs[header->e_shstrndx];
    log::printk(log::log_level::DEBUG,
                "[load_module] shdr_string_table: %p, type: %X, offset: %p\n",
                shdr_string_table, shdr_string_table->sh_type,
                shdr_string_table->sh_offset);
    const char* s_string_table =
        reinterpret_cast<const char*>((mod->sections[header->e_shstrndx]));

    const char* string_table = nullptr;
    size_t modinfo_index     = 0;
    for (uint32_t i = 0; i < header->e_shnum; i++) {
        if (mod->shdrs[i].sh_type == SHT_STRTAB &&
            !libcxx::strcmp(".strtab",
                            s_string_table + mod->shdrs[i].sh_name)) {
            string_table = reinterpret_cast<const char*>(mod->sections[i]);
        } else if (mod->shdrs[i].sh_type == SHT_PROGBITS &&
                   !libcxx::strcmp(".modinfo",
                                   s_string_table + mod->shdrs[i].sh_name)) {
            modinfo_index = i;
        }
    }

    log::printk(log::log_level::DEBUG, "[load_module] String table at %p\n",
                string_table);

    elf::elf_sym* symtab = nullptr;
    size_t num_syms      = 0;
    for (uint32_t i = 0; i < header->e_shnum; i++) {
        if (mod->shdrs[i].sh_type == SHT_SYMTAB) {
            symtab   = reinterpret_cast<elf::elf_sym*>(mod->sections[i]);
            num_syms = mod->shdrs[i].sh_size / mod->shdrs[i].sh_entsize;
            break;
        }
    }

    if (!relocate_module(mod, symtab, string_table)) {
        log::printk(log::log_level::ERROR,
                    "[load_module] Failed to perform relocations\n");
        return false;
    }

    elf::elf_sym* sym = symtab;
    addr_t ctor_start = 0, ctor_end = 0;
    for (size_t j = 1; j <= num_syms; j++, sym++) {
        // We only want to consider functions
        // if (ELF_ST_TYPE(sym->st_info) != STT_FUNC)
        //     continue;
        if (sym->st_shndx == SHN_ABS) {
            continue;
        }
        if (!libcxx::strcmp(string_table + sym->st_name, "init")) {
            log::printk(log::log_level::DEBUG,
                        "[load_module] Located init point at %p\n",
                        mod->sections[sym->st_shndx] + sym->st_value);
            mod->init = reinterpret_cast<int (*)()>(
                mod->sections[sym->st_shndx] + sym->st_value);
        } else if (!libcxx::strcmp(string_table + sym->st_name, "fini")) {
            log::printk(log::log_level::DEBUG,
                        "[load_module] Located fini point at %p\n",
                        mod->sections[sym->st_shndx] + sym->st_value);
            mod->fini = reinterpret_cast<int (*)()>(
                mod->sections[sym->st_shndx] + sym->st_value);
        } else if (!libcxx::strcmp(string_table + sym->st_name,
                                   "__constructors_start")) {
            log::printk(
                log::log_level::DEBUG,
                "[load_module] Located __constructors_start point at %p\n",
                mod->sections[sym->st_shndx] + sym->st_value);
            ctor_start = mod->sections[sym->st_shndx] + sym->st_value;
        } else if (!libcxx::strcmp(string_table + sym->st_name,
                                   "__constructors_end")) {
            log::printk(
                log::log_level::DEBUG,
                "[load_module] Located __constructors_end point at %p\n",
                mod->sections[sym->st_shndx] + sym->st_value);
            ctor_end = mod->sections[sym->st_shndx] + sym->st_value;
        } else {
            if (sym->st_shndx < SHN_LORESERVE) {
                symbols::load_symbol(libcxx::make_pair(
                    string_table + sym->st_name,
                    reinterpret_cast<addr_t>(mod->sections[sym->st_shndx] +
                                             sym->st_value)));
            }
        }
    }

    /*
     * Iterate through the constructors to initialize them
     */
    using constructor_t  = void (*)();
    constructor_t* start = reinterpret_cast<constructor_t*>(ctor_start);
    constructor_t* end   = reinterpret_cast<constructor_t*>(ctor_end);

    for (constructor_t* current = start; current != end; ++current) {
        (*current)();
    }

    // The strings in modinfo are relocated so we need to wait until we
    // completed relocations
    parse_modinfo(mod, modinfo_index);

    if (!mod->name) {
        // Uhh I'm pretty sure this actually leaks
        // TODO: Fix
        log::printk(log::log_level::ERROR,
                    "[load_module]: Missing module name, not loading\n");
        return false;
        delete mod;
    }

    log::printk(log::log_level::INFO, "[load_module] Loaded module %s\n",
                mod->name);
    log::printk(log::log_level::INFO, "[load_module] Description: %s\n",
                (mod->description) ? mod->description : "");
    log::printk(log::log_level::INFO, "[load_module] Author: %s\n",
                (mod->author) ? mod->author : "");
    log::printk(log::log_level::INFO, "[load_module] Version: %s\n",
                (mod->version) ? mod->version : "");

    modules.push_front(*mod);
    mod->init();

    return true;
}

bool unload_module(const char* name)
{
    for (auto it = modules.begin(); it != modules.end(); it++) {
        if (!libcxx::strcmp(it->name, name)) {
            log::printk(log::log_level::INFO,
                        "[unload_module] Unloading module %s\n", name);
            if (it->fini) {
                it->fini();
            }
            delete[] it->shdrs;
            for (size_t i = 0; i < it->shnum; i++) {
                delete[] reinterpret_cast<uint8_t*>(it->sections[i]);
            }
            auto ptr = &(*it);
            delete[] it->sections;
            modules.erase(it);
            delete ptr;
            return true;
        }
    }
    return false;
}
