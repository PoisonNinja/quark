#include <arch/mm/layout.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/elf.h>
#include <proc/process.h>
#include <proc/sched.h>

namespace elf
{
libcxx::pair<bool, addr_t> load(addr_t binary)
{
    process* process = scheduler::get_current_process();
    elf_ehdr* header = reinterpret_cast<elf_ehdr*>(binary);
    if (libcxx::memcmp(header->e_ident, ELFMAG, 4)) {
        log::printk(log::log_level::ERROR,
                    "Binary passed in is not an ELF file!\n");
        return libcxx::pair<bool, addr_t>(false, 0);
    }
    log::printk(log::log_level::DEBUG, "Section header offset: %p\n",
                header->e_shoff);
    log::printk(log::log_level::DEBUG, "Program header offset: %p\n",
                header->e_phoff);
    for (int i = 0; i < header->e_phnum; i++) {
        elf_phdr* phdr = reinterpret_cast<elf_phdr*>(binary + header->e_phoff +
                                                     (header->e_phentsize * i));
        log::printk(log::log_level::DEBUG, "Header type: %X\n", phdr->p_type);
        if (phdr->p_type == PT_LOAD || phdr->p_type == PT_TLS) {
            if (phdr->p_type == PT_TLS) {
                log::printk(log::log_level::DEBUG, "Found TLS section\n");
                auto [found, addr] =
                    process->vma->locate_range(USER_START, phdr->p_memsz);
                if (!found) {
                    log::printk(log::log_level::ERROR,
                                "Failed to locate section\n");
                    return libcxx::pair<bool, addr_t>(false, 0);
                }
                phdr->p_vaddr = addr;
                log::printk(log::log_level::DEBUG,
                            "TLS section will be at %p\n", phdr->p_vaddr);
                process->set_tls_data(phdr->p_paddr, phdr->p_filesz,
                                      phdr->p_memsz, phdr->p_align);
            }
            log::printk(log::log_level::DEBUG, "Flags:            %X\n",
                        phdr->p_flags);
            log::printk(log::log_level::DEBUG, "Offset:           %p\n",
                        phdr->p_offset);
            log::printk(log::log_level::DEBUG, "Virtual address:  %p\n",
                        phdr->p_vaddr);
            log::printk(log::log_level::DEBUG, "Physical address: %p\n",
                        phdr->p_paddr);
            log::printk(log::log_level::DEBUG, "File size:        %p\n",
                        phdr->p_filesz);
            log::printk(log::log_level::DEBUG, "Memory size:      %p\n",
                        phdr->p_memsz);
            log::printk(log::log_level::DEBUG, "Align:            %p\n",
                        phdr->p_align);
            if (!process->vma->add_vmregion(phdr->p_vaddr, phdr->p_memsz)) {
                log::printk(log::log_level::ERROR, "Failed to add section\n");
                return libcxx::pair<bool, addr_t>(false, 0);
            }
            /*
             * Writable by default so kernel can access, we'll update this later
             */
            int flags = PAGE_USER | PAGE_WRITABLE;
            if (!(phdr->p_flags & PF_X)) {
                flags |= PAGE_NX; // Set NX bit if requested
            }
            memory::virt::map_range(phdr->p_vaddr, phdr->p_memsz, flags);

            log::printk(log::log_level::DEBUG,
                        "Copying from %p -> %p, size %X\n",
                        binary + phdr->p_offset, phdr->p_vaddr, phdr->p_filesz);
            libcxx::memcpy(reinterpret_cast<void*>(phdr->p_vaddr),
                           reinterpret_cast<void*>(binary + phdr->p_offset),
                           phdr->p_filesz);
            if (phdr->p_filesz < phdr->p_memsz) {
                log::printk(log::log_level::DEBUG,
                            "Memory size is larger than file size, "
                            "zeroing rest of segment...\n");
                log::printk(log::log_level::DEBUG, "Zeroing %p, size 0x%X\n",
                            phdr->p_filesz + phdr->p_vaddr,
                            phdr->p_memsz - phdr->p_filesz);
                libcxx::memset(
                    reinterpret_cast<void*>(phdr->p_filesz + phdr->p_vaddr), 0,
                    phdr->p_memsz - phdr->p_filesz);
            }
            if (!(phdr->p_flags & PF_W)) {
                // Remove write access if requested
                memory::virt::protect_range(phdr->p_vaddr, phdr->p_memsz,
                                            flags & ~PAGE_WRITABLE);
            }
        }
    }
    log::printk(log::log_level::DEBUG, "Entry point: %p\n", header->e_entry);
    // TODO: More sanity checks
    return libcxx::make_pair(true, header->e_entry);
}
} // namespace elf
