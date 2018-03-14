#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/elf.h>

namespace ELF
{
addr_t add(void* base, size_t offset)
{
    return reinterpret_cast<addr_t>(base) + offset;
}

static char magic[] = {0x7F, 'E', 'L', 'F'};

addr_t load(addr_t binary)
{
    struct elf64_hdr* header = reinterpret_cast<struct elf64_hdr*>(binary);
    if (String::memcmp(header->e_ident, magic, 4)) {
        Log::printk(Log::ERROR, "Binary passed in is not an ELF file!\n");
        return 0;
    }
    Log::printk(Log::DEBUG, "Section header offset: %p\n", header->e_shoff);
    Log::printk(Log::DEBUG, "Program header offset: %p\n", header->e_phoff);
    for (int i = 0; i < header->e_phnum; i++) {
        struct elf64_phdr* phdr = reinterpret_cast<struct elf64_phdr*>(
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
            int flags = PAGE_USER;
            if (phdr->p_flags & PF_W) {
                flags |= PAGE_WRITABLE;
            }
            if (!(phdr->p_flags & PF_X)) {
                flags |= PAGE_NX;
            }
            for (size_t i = 0; i < phdr->p_filesz;
                 i += Memory::Virtual::PAGE_SIZE) {
                Memory::Virtual::map(i + phdr->p_vaddr,
                                     Memory::Physical::allocate(), flags);
                String::memset(reinterpret_cast<void*>(i + phdr->p_vaddr), 0,
                               Memory::Virtual::PAGE_SIZE);
                String::memcpy(
                    reinterpret_cast<void*>(i + phdr->p_vaddr),
                    reinterpret_cast<void*>(binary + i + phdr->p_offset),
                    Memory::Virtual::PAGE_SIZE);
            }
        }
    }
    for (int i = 0; i < header->e_shnum; i++) {
        struct elf64_shdr* shdr = reinterpret_cast<struct elf64_shdr*>(
            binary + header->e_shoff + (header->e_shentsize * i));
        if (shdr->sh_type == SHT_NOBITS) {
            if (!shdr->sh_size)
                continue;
            if (shdr->sh_flags & SHF_ALLOC) {
                Log::printk(Log::DEBUG, "Found .bss section, asking for %p\n",
                            shdr->sh_addr);
                String::memset(reinterpret_cast<void*>(shdr->sh_addr), 0,
                               shdr->sh_size);
            }
        }
    }
    Log::printk(Log::DEBUG, "Entry point: %p\n", header->e_entry);
    // TODO: More sanity checks
    return header->e_entry;
}
}
