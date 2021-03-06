#include <arch/cpu/gdt.h>
#include <lib/string.h>

namespace cpu
{
namespace x86_64
{
namespace gdt
{
namespace
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

constexpr uint8_t flag_long      = (1 << 1);
constexpr uint8_t flag_protected = (1 << 2);
constexpr uint8_t flag_4kib      = (1 << 3);

extern "C" void gdt_load(addr_t);
extern "C" void tss_load();

struct gdt::Entry entries[num_entries];
struct gdt::descriptor gdt = {
    .limit  = sizeof(struct gdt::Entry) * num_entries - 1,
    .offset = reinterpret_cast<addr_t>(&entries),
};

struct TSS::Entry tss = {0};

void set_entry(struct gdt::Entry* entry, uint32_t base, uint32_t limit,
               uint8_t access, uint8_t flags)
{
    entry->limit_low   = limit & 0xFFFF;
    entry->base_low    = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->access      = access;
    entry->limit_high  = (limit >> 16) & 0xF;
    entry->flags       = flags & 0xF;
    entry->base_high   = (base >> 24) & 0xFF;
}

void write_tss(struct gdt::Entry* gdt1, struct gdt::Entry* gdt2,
               struct TSS::Entry* tss)
{
    uint64_t base  = reinterpret_cast<uint64_t>(tss);
    uint32_t limit = sizeof(struct TSS::Entry);
    gdt::set_entry(gdt1, base, limit, 0xE9, 0);
    gdt::set_entry(gdt2, (base >> 48) & 0xFFFF, (base >> 32) & 0xFFFF, 0, 0);
}
} // namespace

void init()
{
    // Null GDT entry
    gdt::set_entry(&entries[0], 0, 0, 0, 0);
    const uint8_t mode_flag = flag_long;
    gdt::set_entry(&entries[1], 0, 0xFFFFF,
                   access_present(1) | access_privilege(0) |
                       access_mandantory(1) | access_code(0, 1),
                   mode_flag | flag_4kib);
    gdt::set_entry(&entries[2], 0, 0xFFFFF,
                   access_present(1) | access_privilege(0) |
                       access_mandantory(1) | access_data(0, 1),
                   mode_flag | flag_4kib);
    /*
     * x86_64 has data then code segments for userspace to comply with
     * syscall/sysret restrictions
     */
    gdt::set_entry(&entries[3], 0, 0xFFFFF,
                   access_present(1) | access_privilege(3) |
                       access_mandantory(1) | access_data(0, 1),
                   mode_flag | flag_4kib);
    gdt::set_entry(&entries[4], 0, 0xFFFFF,
                   access_present(1) | access_privilege(3) |
                       access_mandantory(1) | access_code(0, 1),
                   mode_flag | flag_4kib);
    gdt::write_tss(&entries[5], &entries[6], &tss);
    gdt::gdt_load(reinterpret_cast<addr_t>(&gdt));
    gdt::tss_load();
}
} // namespace gdt

namespace TSS
{
void set_stack(addr_t stack)
{
    gdt::tss.stack0 = stack;
}

addr_t get_stack()
{
    return gdt::tss.stack0;
}
} // namespace TSS
} // namespace x86_64
} // namespace cpu
