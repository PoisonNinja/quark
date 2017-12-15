#include <arch/cpu/idt.h>

namespace IDT
{
static void set_gate(struct IDT::Entry* entry, uint64_t offset,
                     uint16_t selector, uint8_t attributes)
{
    entry->offset_low = offset & 0xFFFF;
    entry->selector = selector;
    entry->zero_one = 0;
    entry->attributes = attributes;
    entry->offset_middle = (offset >> 16) & 0xFFFF;
    entry->offset_high = (offset >> 32) & 0xFFFFFFFF;
    entry->zero_two = 0;
}

void load()
{
}
}
