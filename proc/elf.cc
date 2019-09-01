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
libcxx::pair<bool, addr_t>
load(libcxx::intrusive_ptr<filesystem::descriptor> file)
{
    uint8_t buffer[sizeof(elf_ehdr)];
    file->pread(buffer, sizeof(elf_ehdr), 0);
    process* process = scheduler::get_current_process();
    elf_ehdr* header = reinterpret_cast<elf_ehdr*>(buffer);
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
        uint8_t phdr_buffer[sizeof(elf_phdr)];
        file->pread(phdr_buffer, sizeof(elf_phdr),
                    header->e_phoff + (header->e_phentsize * i));
        elf_phdr* phdr = reinterpret_cast<elf_phdr*>(phdr_buffer);
        log::printk(log::log_level::DEBUG, "Header type: %X\n", phdr->p_type);
        if (phdr->p_type == PT_TLS) {
            log::printk(log::log_level::DEBUG, "Found TLS section\n");
            log::printk(log::log_level::DEBUG, "TLS section will be at %p\n",
                        phdr->p_vaddr);
            process->set_tls_data(phdr->p_offset, phdr->p_filesz, phdr->p_memsz,
                                  phdr->p_align);
        }
        if (phdr->p_type == PT_LOAD) {
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
            process->mmap(phdr->p_vaddr, phdr->p_memsz, PROT_WRITE | PROT_EXEC,
                          MAP_FILE | MAP_PRIVATE | MAP_FIXED, file,
                          phdr->p_offset);
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
        }
    }
    log::printk(log::log_level::DEBUG, "Entry point: %p\n", header->e_entry);
    // TODO: More sanity checks
    return libcxx::make_pair(true, header->e_entry);
}
} // namespace elf
