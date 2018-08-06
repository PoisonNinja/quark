#include <kernel.h>
#include <kernel/module.h>
#include <kernel/symbol.h>
#include <proc/elf.h>

bool relocate_module(Module* mod, ELF::Elf_Sym* symtab,
                     const char* string_table)
{
    for (uint32_t i = 0; i < mod->shnum; i++) {
#ifdef X86_64
        if (mod->shdrs[i].sh_type == SHT_RELA) {
#else
        if (mod->shdrs[i].sh_type == SHT_REL) {
#endif
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

                addr_t addend;
#ifdef X86_64
                addend = rel->r_addend;
#else
                addend = (*(uint32_t*)(target));
#endif

                switch (ELF_R_TYPE(rel->r_info)) {
                    case R_X86_64_64:
                        Log::printk(Log::LogLevel::DEBUG,
                                    "[load_module] R_X86_64_64: %p %p %p %X\n",
                                    addend, rel->r_offset, symaddr,
                                    mod->shdrs[i].sh_info);
                        *(reinterpret_cast<addr_t*>(target)) = symaddr + addend;
                        break;
                    case R_386_PC32:
                        Log::printk(Log::LogLevel::DEBUG,
                                    "[load_module] R_386_PC32: %p %p %p %X\n",
                                    addend, target, symaddr,
                                    mod->shdrs[i].sh_info);
                        *(reinterpret_cast<addr_t*>(target)) =
                            symaddr + addend - target;
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
    return true;
}
