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

void load(addr_t binary, Thread* thread)
{
    struct elf64_hdr* header = reinterpret_cast<struct elf64_hdr*>(binary);
    if (String::memcmp(header->e_ident, magic, 4)) {
        Log::printk(Log::ERROR, "Binary passed in is not an ELF file!\n");
        return;
    }
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
            for (size_t i = 0; i < phdr->p_filesz;
                 i += Memory::Virtual::PAGE_SIZE) {
                Memory::Virtual::map(i + phdr->p_vaddr, Memory::Physical::get(),
                                     PAGE_USER | PAGE_WRITABLE);
                String::memset(reinterpret_cast<void*>(i + phdr->p_vaddr), 0,
                               Memory::Virtual::PAGE_SIZE);
                String::memcpy(
                    reinterpret_cast<void*>(i + phdr->p_vaddr),
                    reinterpret_cast<void*>(binary + i + phdr->p_offset),
                    Memory::Virtual::PAGE_SIZE);
            }
        }
    }
    addr_t phys_stack = Memory::Physical::get();
    Memory::Virtual::map(0x1000, phys_stack,
                         PAGE_WRITABLE | PAGE_USER);
    String::memset((void*)0x1000, 0, 4096);
    String::memset(&thread->cpu_ctx, 0, sizeof(thread->cpu_ctx));
    Log::printk(Log::DEBUG, "Entry point: %p\n", header->e_entry);
    thread->cpu_ctx.rip = header->e_entry;
    thread->cpu_ctx.cs = 0x18 | 3;
    thread->cpu_ctx.ds = 0x20 | 3;
    thread->cpu_ctx.ss = 0x20 | 3;
    thread->cpu_ctx.rsp = thread->cpu_ctx.rbp = 0x2000;
    thread->cpu_ctx.rflags = 0x200;
    thread->kernel_stack =
        reinterpret_cast<addr_t>(new uint8_t[0x1000]) + 0x1000;
    // TODO: More sanity checks
}
}
