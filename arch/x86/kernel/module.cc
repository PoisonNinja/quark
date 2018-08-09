#include <kernel.h>
#include <kernel/module.h>
#include <kernel/symbol.h>
#include <proc/elf.h>

bool relocate_module(Module* mod, ELF::Elf_Sym* symtab,
                     const char* string_table)
{
    for (uint32_t i = 0; i < mod->shnum; i++) {
        /*
         * Modern platforms (x86_64 included) use RELA, but unfortunately i686
         * is stuck in the past. rela adds an explicit r_addend member whereas
         * rel has an implicit addend.
         */
#ifdef X86_64
        if (mod->shdrs[i].sh_type == SHT_RELA) {
#else
        if (mod->shdrs[i].sh_type == SHT_REL) {
#endif
            for (uint32_t x = 0; x < mod->shdrs[i].sh_size;
                 x += mod->shdrs[i].sh_entsize) {
                /*
                 * Even though i686 uses rel, not rela, we can use rela for
                 * both as long as we don't use r_addend for i686 since it's
                 * the last member.
                 */
                ELF::Elf_Rela* rel =
                    reinterpret_cast<ELF::Elf_Rela*>(mod->sections[i] + x);
                // Get the symbol offset
                ELF::Elf_Sym* sym = symtab + ELF_R_SYM(rel->r_info);
                Log::printk(Log::LogLevel::DEBUG, "[load_module] Name: %s\n",
                            string_table + sym->st_name);
                addr_t symaddr = 0;
                /*
                 * If st_shndx is 0, we need to resolve it to a global/external
                 * symbol. Otherwise, it's an internal reference that we need
                 * to resolve
                 */
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
                    /*
                     * Internal reference. st_shndx stores the index of the
                     * section containing the target symbol and st_value
                     * contains the offset.
                     */
                    symaddr = mod->sections[sym->st_shndx] + sym->st_value;
                }
                // Get the target section from sh_info
                addr_t target = mod->sections[mod->shdrs[i].sh_info];
                // Add the offset into the section
                target += rel->r_offset;

                addr_t addend;
#ifdef X86_64
                addend = rel->r_addend;
#else
                // The addend is stored at the target itself
                addend = (*(uint32_t*)(target));
#endif

                switch (ELF_R_TYPE(rel->r_info)) {
                    // case R_386_32
                    case R_X86_64_64:
                        Log::printk(Log::LogLevel::DEBUG,
                                    "[load_module] R_X86_64_64: %p %p %p %X\n",
                                    addend, rel->r_offset, symaddr,
                                    mod->shdrs[i].sh_info);
                        // = S + A
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
