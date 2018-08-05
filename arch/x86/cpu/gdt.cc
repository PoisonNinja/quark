#include <arch/cpu/gdt.h>
#include <lib/string.h>

namespace CPU
{
namespace X86
{
namespace GDT
{
#ifdef X86_64
constexpr size_t num_entries = 7;
#else
constexpr size_t num_entries = 8;
#endif

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

constexpr uint8_t flag_long = (1 << 1);
constexpr uint8_t flag_protected = (1 << 2);
constexpr uint8_t flag_4kib = (1 << 3);

extern "C" void gdt_load(addr_t);
extern "C" void tss_load();

static struct GDT::Entry entries[num_entries];
static struct GDT::Descriptor descriptor = {
    .limit = sizeof(struct GDT::Entry) * num_entries - 1,
    .offset = reinterpret_cast<addr_t>(&entries),
};

#ifdef X86_64
static struct TSS::Entry tss = {
    .reserved0 = 0,
    .stack0 = 0,
    .stack1 = 0,
    .stack2 = 0,
    .reserved1 = 0,
    .ist = {0, 0, 0, 0, 0, 0, 0},
    .reserved2 = 0,
    .reserved3 = 0,
    .iomap_base = 0,
};
#else
static struct TSS::Entry tss = {0};
#endif

static void set_entry(struct GDT::Entry* entry, uint32_t base, uint32_t limit,
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

#ifdef X86_64
static void write_tss(struct GDT::Entry* gdt1, struct GDT::Entry* gdt2,
                      struct TSS::Entry* tss)
{
    uint64_t base = (uint64_t)tss;
    uint32_t limit = sizeof(struct TSS::Entry);
    GDT::set_entry(gdt1, base, limit, 0xE9, 0);
    GDT::set_entry(gdt2, (base >> 48) & 0xFFFF, (base >> 32) & 0xFFFF, 0, 0);
}
#else
static void set_entry_base(struct GDT::Entry* entry, uint32_t base)
{
    entry->base_low = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->base_high = (base >> 24) & 0xFF;
}

static uint32_t get_entry_base(struct GDT::Entry* entry)
{
    return entry->base_low | entry->base_middle << 16 | entry->base_high << 24;
}

static void write_tss(struct GDT::Entry* gdt, struct TSS::Entry* tss)
{
    uint32_t base = (uint32_t)tss;
    uint16_t limit = sizeof(struct TSS::Entry) - 1;
    GDT::set_entry(gdt, base, limit, 0xE9, 0);
}
#endif

void init()
{
    GDT::set_entry(&entries[0], 0, 0, 0, 0);
#ifdef X86_64
    const uint8_t mode_flag = flag_long;
#else
    const uint8_t mode_flag = flag_protected;
#endif
    GDT::set_entry(&entries[1], 0, 0xFFFFF,
                   access_present(1) | access_privilege(0) |
                       access_mandantory(1) | access_code(0, 1),
                   mode_flag | flag_4kib);
    GDT::set_entry(&entries[2], 0, 0xFFFFF,
                   access_present(1) | access_privilege(0) |
                       access_mandantory(1) | access_data(0, 1),
                   mode_flag | flag_4kib);
    GDT::set_entry(&entries[3], 0, 0xFFFFF,
                   access_present(1) | access_privilege(3) |
                       access_mandantory(1) | access_data(0, 1),
                   mode_flag | flag_4kib);
    GDT::set_entry(&entries[4], 0, 0xFFFFF,
                   access_present(1) | access_privilege(3) |
                       access_mandantory(1) | access_code(0, 1),
                   mode_flag | flag_4kib);
#ifdef X86_64
    GDT::write_tss(&entries[5], &entries[6], &tss);
#else
    GDT::write_tss(&entries[5], &tss);
    // F segment
    GDT::set_entry(&entries[6], 0, 0xFFFFF,
                   access_present(1) | access_privilege(3) |
                       access_mandantory(1) | access_data(0, 1),
                   mode_flag | flag_4kib);
    // G segment
    GDT::set_entry(&entries[7], 0, 0xFFFFF,
                   access_present(1) | access_privilege(3) |
                       access_mandantory(1) | access_data(0, 1),
                   mode_flag | flag_4kib);
    tss.ss0 = 0x10;
#endif
    GDT::gdt_load(reinterpret_cast<addr_t>(&descriptor));
    GDT::tss_load();
}

#ifndef X86_64
addr_t get_fs()
{
    return get_entry_base(&entries[6]);
}

addr_t get_gs()
{
    return get_entry_base(&entries[7]);
}

void set_fs(addr_t base)
{
    set_entry_base(&entries[6], base);
    asm volatile("mov %0, %%fs" : : "r"(0x33));
}

void set_gs(addr_t base)
{
    set_entry_base(&entries[7], base);
    asm volatile("mov %0, %%gs" : : "r"(0x3B));
}
#endif
}  // namespace GDT

namespace TSS
{
void set_stack(addr_t stack)
{
#ifdef X86_64
    GDT::tss.stack0 = stack;
#else
    GDT::tss.esp0 = stack;
#endif
}

addr_t get_stack()
{
#ifdef X86_64
    return GDT::tss.stack0;
#else
    return GDT::tss.esp0;
#endif
}
}  // namespace TSS
}  // namespace X86
}  // namespace CPU
