#include <arch/mm/layout.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/elf.h>
#include <proc/process.h>
#include <proc/sched.h>

namespace ELF
{
addr_t load(addr_t binary)
{
    Process* process = Scheduler::get_current_process();
    Elf_Ehdr* header = reinterpret_cast<Elf_Ehdr*>(binary);
    if (String::memcmp(header->e_ident, ELFMAG, 4)) {
        Log::printk(Log::ERROR, "Binary passed in is not an ELF file!\n");
        return 0;
    }
    Log::printk(Log::DEBUG, "Section header offset: %p\n", header->e_shoff);
    Log::printk(Log::DEBUG, "Program header offset: %p\n", header->e_phoff);
    for (int i = 0; i < header->e_phnum; i++) {
        Elf_Phdr* phdr = reinterpret_cast<Elf_Phdr*>(binary + header->e_phoff +
                                                     (header->e_phentsize * i));
        Log::printk(Log::DEBUG, "Header type: %X\n", phdr->p_type);
        if (phdr->p_type == PT_LOAD || phdr->p_type == PT_TLS) {
            if (phdr->p_type == PT_TLS) {
                Log::printk(Log::DEBUG, "Found TLS section\n");
                if (!process->sections->locate_range(phdr->p_vaddr, USER_START,
                                                     phdr->p_memsz)) {
                    Log::printk(Log::ERROR, "Failed to locate section\n");
                    return 0;
                }
                Log::printk(Log::DEBUG, "TLS section will be at %p\n",
                            phdr->p_vaddr);
            }
            Log::printk(Log::DEBUG, "Flags:            %X\n", phdr->p_flags);
            Log::printk(Log::DEBUG, "Offset:           %p\n", phdr->p_offset);
            Log::printk(Log::DEBUG, "Virtual address:  %p\n", phdr->p_vaddr);
            Log::printk(Log::DEBUG, "Physical address: %p\n", phdr->p_paddr);
            Log::printk(Log::DEBUG, "File size:        %p\n", phdr->p_filesz);
            Log::printk(Log::DEBUG, "Memory size:      %p\n", phdr->p_memsz);
            Log::printk(Log::DEBUG, "Align:            %p\n", phdr->p_align);
            if (!process->sections->add_section(phdr->p_vaddr, phdr->p_memsz)) {
                Log::printk(Log::ERROR, "Failed to add section\n");
                return 0;
            }
            int flags = PAGE_USER | PAGE_WRITABLE; /*
                                                    * Writable by default so
                                                    * kernel can access, we'll
                                                    * update this later
                                                    */
            if (!(phdr->p_flags & PF_X)) {
                flags |= PAGE_NX;  // Set NX bit if requested
            }
            Memory::Virtual::map_range(phdr->p_vaddr, phdr->p_memsz, flags);

            Log::printk(Log::DEBUG, "Copying from %p -> %p, size %X\n",
                        binary + phdr->p_offset, phdr->p_vaddr, phdr->p_filesz);
            String::memcpy(reinterpret_cast<void*>(phdr->p_vaddr),
                           reinterpret_cast<void*>(binary + phdr->p_offset),
                           phdr->p_filesz);
            if (phdr->p_filesz < phdr->p_memsz) {
                Log::printk(Log::DEBUG, "Memory size is larger than file size, "
                                        "zeroing rest of segment...\n");
                Log::printk(Log::DEBUG, "Zeroing %p, size 0x%X\n",
                            phdr->p_filesz + phdr->p_vaddr,
                            phdr->p_memsz - phdr->p_filesz);
                String::memset(
                    reinterpret_cast<void*>(phdr->p_filesz + phdr->p_vaddr), 0,
                    phdr->p_memsz - phdr->p_filesz);
            }
            if (!(phdr->p_flags & PF_W)) {
                // Remove write access if requested
                Memory::Virtual::protect_range(phdr->p_vaddr, phdr->p_memsz,
                                               flags & ~PAGE_WRITABLE);
            }
        }
    }
    Log::printk(Log::DEBUG, "Entry point: %p\n", header->e_entry);
    // TODO: More sanity checks
    return header->e_entry;
}  // namespace ELF
}  // namespace ELF
