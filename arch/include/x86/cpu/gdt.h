#pragma once

#include <types.h>

namespace cpu
{
namespace X86
{
namespace GDT
{
struct Descriptor {
    uint16_t limit;
    addr_t offset;
} __attribute__((packed));

struct Entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_high : 4;
    uint8_t flags : 4;
    uint8_t base_high;
} __attribute__((packed));

#ifndef X86_64
addr_t get_fs();
addr_t get_gs();

void set_fs(addr_t base);
void set_gs(addr_t base);
#endif

void init();
}  // namespace GDT

namespace TSS
{
#ifdef X86_64
struct Entry {
    uint32_t reserved0;
    uint64_t stack0; /* This is not naturally aligned, so packed is needed. */
    uint64_t stack1;
    uint64_t stack2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));
#else
struct Entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));
#endif

void set_stack(addr_t stack);
addr_t get_stack();
}  // namespace TSS
}  // namespace X86
}  // namespace cpu
