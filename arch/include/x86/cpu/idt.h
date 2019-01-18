#pragma once

#include <types.h>

namespace cpu
{
namespace X86
{
namespace IDT
{
struct Descriptor {
    uint16_t limit;
    addr_t offset;
} __attribute__((packed));

struct Entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t attributes;
    uint16_t offset_middle;
#ifdef X86_64
    uint32_t offset_high;
    uint32_t zero_two;
#endif
} __attribute__((packed));

void init();
}  // namespace IDT
}  // namespace X86
}  // namespace cpu
