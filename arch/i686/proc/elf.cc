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
    struct elf32_hdr* header = reinterpret_cast<struct elf32_hdr*>(binary);
    if (String::memcmp(header->e_ident, ELFMAG, 4)) {
        Log::printk(Log::ERROR, "Binary passed in is not an ELF file!\n");
        return 0;
    }
    Log::printk(Log::DEBUG, "Section header offset: %p\n", header->e_shoff);
    Log::printk(Log::DEBUG, "Program header offset: %p\n", header->e_phoff);
    for (int i = 0; i < header->e_phnum; i++) {
        struct elf32_phdr* phdr = reinterpret_cast<struct elf32_phdr*>(
            binary + header->e_phoff + (header->e_phentsize * i));
        Log::printk(Log::DEBUG, "Header type: %X\n", phdr->p_type);
        if (phdr->p_type == PT_LOAD) {
            Log::printk(Log::DEBUG, "Flags:            %X\n", phdr->p_flags);
            Log::printk(Log::DEBUG, "Offset:           %p\n", phdr->p_offset);
            Log::printk(Log::DEBUG, "Virtual address:  %p\n", phdr->p_vaddr);
            Log::printk(Log::DEBUG, "Physical address: %p\n", phdr->p_paddr);
            Log::printk(Log::DEBUG, "File size:        %llX\n", phdr->p_filesz);
            Log::printk(Log::DEBUG, "Memory size:      %llX\n", phdr->p_memsz);
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
            size_t i;
            for (i = 0; i < phdr->p_filesz;) {
                Memory::Virtual::map(i + phdr->p_vaddr,
                                     Memory::Physical::allocate(), flags);
                /*
                 * Calculate how much to copy for this segment.
                 *
                 * Add one to i + phdr->p_vaddr to force it to round upwards if
                 * i + phdr->p_vaddr is a multiple of 4096
                 */
                size_t size =
                    (Memory::Virtual::align_up(i + phdr->p_vaddr + 1) >
                     phdr->p_vaddr + phdr->p_filesz) ?
                        phdr->p_filesz - i :
                        Memory::Virtual::align_up(i + phdr->p_vaddr + 1) -
                            (i + phdr->p_vaddr);
                Log::printk(Log::DEBUG, "Copying from %p -> %p, size %X\n",
                            binary + i + phdr->p_offset, i + phdr->p_vaddr,
                            size);
                String::memcpy(
                    reinterpret_cast<void*>(i + phdr->p_vaddr),
                    reinterpret_cast<void*>(binary + i + phdr->p_offset), size);
                if (!(phdr->p_flags & PF_W)) {
                    // Remove write access if requested
                    Memory::Virtual::protect(i + phdr->p_vaddr,
                                             flags & ~PAGE_WRITABLE);
                }
                i += size;
            }
            if (i < phdr->p_memsz) {
                Log::printk(Log::DEBUG, "Memory size is larger than file size, "
                                        "zeroing...\n");
                while (i < phdr->p_memsz) {
                    if (Memory::Virtual::test(i + phdr->p_vaddr)) {
                        Memory::Virtual::protect(i + phdr->p_vaddr, flags);
                    } else {
                        Memory::Virtual::map(i + phdr->p_vaddr,
                                             Memory::Physical::allocate(),
                                             flags);
                    }
                    size_t size =
                        (Memory::Virtual::align_up(i + phdr->p_vaddr + 1) >
                         phdr->p_vaddr + phdr->p_memsz) ?
                            phdr->p_memsz - i :
                            Memory::Virtual::align_up(i + phdr->p_vaddr + 1) -
                                (i + phdr->p_vaddr);
                    Log::printk(Log::DEBUG, "Zeroing %p, size 0x%X\n",
                                i + phdr->p_vaddr, size);
                    String::memset(reinterpret_cast<void*>(i + phdr->p_vaddr),
                                   0, size);
                    if (!(phdr->p_flags & PF_W)) {
                        Memory::Virtual::protect(i + phdr->p_vaddr,
                                                 flags & ~PAGE_WRITABLE);
                    }
                    i += size;
                }
            }
        }
    }
    Log::printk(Log::DEBUG, "Entry point: %p\n", header->e_entry);
    // TODO: More sanity checks
    return header->e_entry;
}
}
