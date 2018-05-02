#pragma once

#include <types.h>

namespace CPU
{
namespace X86
{
namespace IDT
{
struct Descriptor {
    uint16_t limit;
    uint32_t offset;
} __attribute__((packed));

struct Entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t attributes;
    uint16_t offset_high;
} __attribute__((packed));

void init();
}  // namespace IDT
}
}
