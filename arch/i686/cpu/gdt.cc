#include <arch/cpu/gdt.h>
#include <lib/string.h>

namespace CPU
{
namespace X86
{
namespace GDT
{
constexpr size_t num_entries = 7;

constexpr uint8_t access_present(uint8_t x)
{
    return ((x) << 7);
}

constexpr uint8_t access_privilege(uint8_t x)
{
    return ((x) << 5);
}

constexpr uint8_t access_mandantory(uint8_t x)
{
    return ((x) << 4);
}

constexpr uint8_t access_data(uint8_t direction, uint8_t write)
{
    return ((0) << 3 | (direction) << 2 | (write) << 1);
}

constexpr uint8_t access_code(uint8_t conform, uint8_t read)
{
    return ((1) << 3 | (conform) << 2 | (read) << 1);
}

constexpr uint8_t flag_protected = (1 << 2);
constexpr uint8_t flag_long = (1 << 1);
constexpr uint8_t flag_4kib = (1 << 3);

extern "C" void gdt_load(addr_t);
extern "C" void tss_load();

static struct GDT::Entry entries[num_entries];
static struct GDT::Descriptor descriptor = {
    .limit = sizeof(struct GDT::Entry) * num_entries - 1,
    .offset = reinterpret_cast<addr_t>(&entries),
};
static struct TSS::Entry tss = {
    // .reserved0 = 0,
    // .stack0 = 0,
    // .stack1 = 0,
    // .stack2 = 0,
    // .reserved1 = 0,
    // .ist = {0, 0, 0, 0, 0, 0, 0},
    // .reserved2 = 0,
    // .reserved3 = 0,
    // .iomap_base = 0,
};

static void set_entry(struct GDT::Entry* entry, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t flags)
{
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->access = access;
    entry->flags = flags & 0xF;
    entry->base_high = (base >> 24) & 0xFF;
}

static void write_tss(struct GDT::Entry* gdt1, struct GDT::Entry* gdt2,
                      struct TSS::Entry* tss)
{
    uint64_t base = (uint64_t)tss;
    uint32_t limit = sizeof(struct TSS::Entry);
    GDT::set_entry(gdt1, base, limit, 0xE9, 0);
    GDT::set_entry(gdt2, (base >> 48) & 0xFFFF, (base >> 32) & 0xFFFF, 0, 0);
}

void init()
{
    GDT::set_entry(&entries[0], 0, 0, 0, 0);
    GDT::set_entry(&entries[1], 0, 0xFFFFF,
                   access_present(1) | access_privilege(0) |
                       access_mandantory(1) | access_code(0, 1),
                   flag_protected | flag_4kib);
    GDT::set_entry(&entries[2], 0, 0xFFFFF,
                   access_present(1) | access_privilege(0) |
                       access_mandantory(1) | access_data(0, 1),
                   flag_protected | flag_4kib);
    GDT::set_entry(&entries[3], 0, 0xFFFFF,
                   access_present(1) | access_privilege(3) |
                       access_mandantory(1) | access_data(0, 1),
                   flag_protected | flag_4kib);
    GDT::set_entry(&entries[4], 0, 0xFFFFF,
                   access_present(1) | access_privilege(3) |
                       access_mandantory(1) | access_code(0, 1),
                   flag_protected | flag_4kib);
    GDT::write_tss(&entries[5], &entries[6], &tss);
    GDT::gdt_load(reinterpret_cast<addr_t>(&descriptor));
    GDT::tss_load();
}
}  // namespace GDT

namespace TSS
{
void set_stack(addr_t stack)
{
    // GDT::tss.stack0 = stack;
}

addr_t get_stack()
{
    // return GDT::tss.stack0;
}
}
}
}
