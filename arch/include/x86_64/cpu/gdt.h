#pragma once

#include <types.h>

namespace cpu
{
namespace x86
{
namespace gdt
{
struct descriptor {
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
} // namespace gdt

namespace TSS
{
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

void set_stack(addr_t stack);
addr_t get_stack();
} // namespace TSS
} // namespace x86
} // namespace cpu
