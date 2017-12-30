#include <arch/cpu/gdt.h>
#include <lib/string.h>

namespace GDT
{
#define NUM_ENTRIES 5

#define ACCESS_PRESENT(x) ((x) << 7)
#define ACCESS_PRIVL(x) ((x) << 5)
#define ACCESS_MANDATORY(x) ((x) << 4)
#define ACCESS_DATA(direction, write) \
    ((0) << 3 | (direction) << 2 | (write) << 1)
#define ACCESS_CODE(conform, read) ((1) << 3 | (conform) << 2 | (read) << 1)

#define FLAG_LONG (1 << 1)
#define FLAG_4KIB (1 << 3)

extern "C" void gdt_load(addr_t);

static struct GDT::Entry entries[NUM_ENTRIES];
static struct GDT::Descriptor descriptor = {
    .limit = sizeof(struct GDT::Entry) * NUM_ENTRIES - 1,
    .offset = reinterpret_cast<addr_t>(&entries),
};

static void set_entry(struct GDT::Entry *entry, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t flags)
{
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->access = access;
    entry->limit_high = (limit >> 16) & 0xF;
    entry->flags = flags & 0xF;
    entry->base_high = (base >> 24) & 0xFF;
}

void init()
{
    GDT::set_entry(&entries[0], 0, 0, 0, 0);
    GDT::set_entry(&entries[1], 0, 0xFFFFF,
                   ACCESS_PRESENT(1) | ACCESS_PRIVL(0) | ACCESS_MANDATORY(1) |
                       ACCESS_CODE(0, 1),
                   FLAG_LONG | FLAG_4KIB);
    GDT::set_entry(&entries[2], 0, 0xFFFFF,
                   ACCESS_PRESENT(1) | ACCESS_PRIVL(0) | ACCESS_MANDATORY(1) |
                       ACCESS_DATA(0, 1),
                   FLAG_LONG | FLAG_4KIB);
    GDT::set_entry(&entries[3], 0, 0xFFFFF,
                   ACCESS_PRESENT(1) | ACCESS_PRIVL(3) | ACCESS_MANDATORY(1) |
                       ACCESS_CODE(0, 1),
                   FLAG_LONG | FLAG_4KIB);
    GDT::set_entry(&entries[4], 0, 0xFFFFF,
                   ACCESS_PRESENT(1) | ACCESS_PRIVL(3) | ACCESS_MANDATORY(1) |
                       ACCESS_DATA(0, 1),
                   FLAG_LONG | FLAG_4KIB);
    GDT::gdt_load(reinterpret_cast<addr_t>(&descriptor));
}
}  // namespace GDT
