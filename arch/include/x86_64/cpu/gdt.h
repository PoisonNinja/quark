#pragma once

#include <types.h>

namespace GDT
{
struct Descriptor {
    uint16_t limit;
    uint64_t offset;
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

void init();
}  // namespace GDT
