#pragma once

#include <types.h>

namespace IDT
{
struct Descriptor {
    uint16_t limit;
    uint64_t offset;
} __attribute__((packed));

struct Entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero_one;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t zero_two;
} __attribute__((packed));

void load();
}
