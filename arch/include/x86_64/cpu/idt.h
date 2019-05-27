#pragma once

#include <types.h>

namespace cpu
{
namespace x86_64
{
namespace idt
{
struct descriptor {
    uint16_t limit;
    addr_t offset;
} __attribute__((packed));

struct Entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t zero_two;
} __attribute__((packed));

void init();
} // namespace idt
} // namespace x86_64
} // namespace cpu
