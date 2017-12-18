#include <arch/cpu/idt.h>

namespace IDT
{
#define NUM_ENTRIES 256

static struct IDT::Entry entries[NUM_ENTRIES];
static struct IDT::Descriptor descriptor = {
    .limit = sizeof(struct IDT::Entry) * NUM_ENTRIES - 1,
    .offset = reinterpret_cast<addr_t>(&entries),
};

extern "C" void isr0(void);
extern "C" void isr1(void);
extern "C" void isr2(void);
extern "C" void isr3(void);
extern "C" void isr4(void);
extern "C" void isr5(void);
extern "C" void isr6(void);
extern "C" void isr7(void);
extern "C" void isr8(void);
extern "C" void isr9(void);
extern "C" void isr10(void);
extern "C" void isr11(void);
extern "C" void isr12(void);
extern "C" void isr13(void);
extern "C" void isr14(void);
extern "C" void isr15(void);
extern "C" void isr16(void);
extern "C" void isr17(void);
extern "C" void isr18(void);
extern "C" void isr19(void);
extern "C" void isr20(void);
extern "C" void isr21(void);
extern "C" void isr22(void);
extern "C" void isr23(void);
extern "C" void isr24(void);
extern "C" void isr25(void);
extern "C" void isr26(void);
extern "C" void isr27(void);
extern "C" void isr28(void);
extern "C" void isr29(void);
extern "C" void isr30(void);
extern "C" void isr31(void);
extern "C" void irq0(void);
extern "C" void irq1(void);
extern "C" void irq2(void);
extern "C" void irq3(void);
extern "C" void irq4(void);
extern "C" void irq5(void);
extern "C" void irq6(void);
extern "C" void irq7(void);
extern "C" void irq8(void);
extern "C" void irq9(void);
extern "C" void irq10(void);
extern "C" void irq11(void);
extern "C" void irq12(void);
extern "C" void irq13(void);
extern "C" void irq14(void);
extern "C" void irq15(void);

extern "C" void idt_load(uint64_t);

static void set_entry(struct IDT::Entry* entry, uint64_t offset,
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

void init()
{
    IDT::set_entry(&entries[0], reinterpret_cast<uint64_t>(isr0), 0x08, 0x8E);
    IDT::set_entry(&entries[1], reinterpret_cast<uint64_t>(isr1), 0x08, 0x8E);
    IDT::set_entry(&entries[2], reinterpret_cast<uint64_t>(isr2), 0x08, 0x8E);
    IDT::set_entry(&entries[3], reinterpret_cast<uint64_t>(isr3), 0x08, 0x8E);
    IDT::set_entry(&entries[4], reinterpret_cast<uint64_t>(isr4), 0x08, 0x8E);
    IDT::set_entry(&entries[5], reinterpret_cast<uint64_t>(isr5), 0x08, 0x8E);
    IDT::set_entry(&entries[6], reinterpret_cast<uint64_t>(isr6), 0x08, 0x8E);
    IDT::set_entry(&entries[7], reinterpret_cast<uint64_t>(isr7), 0x08, 0x8E);
    IDT::set_entry(&entries[8], reinterpret_cast<uint64_t>(isr8), 0x08, 0x8E);
    IDT::set_entry(&entries[9], reinterpret_cast<uint64_t>(isr9), 0x08, 0x8E);
    IDT::set_entry(&entries[10], reinterpret_cast<uint64_t>(isr10), 0x08, 0x8E);
    IDT::set_entry(&entries[11], reinterpret_cast<uint64_t>(isr11), 0x08, 0x8E);
    IDT::set_entry(&entries[12], reinterpret_cast<uint64_t>(isr12), 0x08, 0x8E);
    IDT::set_entry(&entries[13], reinterpret_cast<uint64_t>(isr13), 0x08, 0x8E);
    IDT::set_entry(&entries[14], reinterpret_cast<uint64_t>(isr14), 0x08, 0x8E);
    IDT::set_entry(&entries[15], reinterpret_cast<uint64_t>(isr15), 0x08, 0x8E);
    IDT::set_entry(&entries[16], reinterpret_cast<uint64_t>(isr16), 0x08, 0x8E);
    IDT::set_entry(&entries[17], reinterpret_cast<uint64_t>(isr17), 0x08, 0x8E);
    IDT::set_entry(&entries[18], reinterpret_cast<uint64_t>(isr18), 0x08, 0x8E);
    IDT::set_entry(&entries[19], reinterpret_cast<uint64_t>(isr19), 0x08, 0x8E);
    IDT::set_entry(&entries[20], reinterpret_cast<uint64_t>(isr20), 0x08, 0x8E);
    IDT::set_entry(&entries[21], reinterpret_cast<uint64_t>(isr21), 0x08, 0x8E);
    IDT::set_entry(&entries[22], reinterpret_cast<uint64_t>(isr22), 0x08, 0x8E);
    IDT::set_entry(&entries[23], reinterpret_cast<uint64_t>(isr23), 0x08, 0x8E);
    IDT::set_entry(&entries[24], reinterpret_cast<uint64_t>(isr24), 0x08, 0x8E);
    IDT::set_entry(&entries[25], reinterpret_cast<uint64_t>(isr25), 0x08, 0x8E);
    IDT::set_entry(&entries[26], reinterpret_cast<uint64_t>(isr26), 0x08, 0x8E);
    IDT::set_entry(&entries[27], reinterpret_cast<uint64_t>(isr27), 0x08, 0x8E);
    IDT::set_entry(&entries[28], reinterpret_cast<uint64_t>(isr28), 0x08, 0x8E);
    IDT::set_entry(&entries[29], reinterpret_cast<uint64_t>(isr29), 0x08, 0x8E);
    IDT::set_entry(&entries[30], reinterpret_cast<uint64_t>(isr30), 0x08, 0x8E);
    IDT::set_entry(&entries[31], reinterpret_cast<uint64_t>(isr31), 0x08, 0x8E);
    IDT::set_entry(&entries[32], reinterpret_cast<uint64_t>(irq0), 0x08, 0x8E);
    IDT::set_entry(&entries[33], reinterpret_cast<uint64_t>(irq1), 0x08, 0x8E);
    IDT::set_entry(&entries[34], reinterpret_cast<uint64_t>(irq2), 0x08, 0x8E);
    IDT::set_entry(&entries[35], reinterpret_cast<uint64_t>(irq3), 0x08, 0x8E);
    IDT::set_entry(&entries[36], reinterpret_cast<uint64_t>(irq4), 0x08, 0x8E);
    IDT::set_entry(&entries[37], reinterpret_cast<uint64_t>(irq5), 0x08, 0x8E);
    IDT::set_entry(&entries[38], reinterpret_cast<uint64_t>(irq6), 0x08, 0x8E);
    IDT::set_entry(&entries[39], reinterpret_cast<uint64_t>(irq7), 0x08, 0x8E);
    IDT::set_entry(&entries[40], reinterpret_cast<uint64_t>(irq8), 0x08, 0x8E);
    IDT::set_entry(&entries[41], reinterpret_cast<uint64_t>(irq9), 0x08, 0x8E);
    IDT::set_entry(&entries[42], reinterpret_cast<uint64_t>(irq10), 0x08, 0x8E);
    IDT::set_entry(&entries[43], reinterpret_cast<uint64_t>(irq11), 0x08, 0x8E);
    IDT::set_entry(&entries[44], reinterpret_cast<uint64_t>(irq12), 0x08, 0x8E);
    IDT::set_entry(&entries[45], reinterpret_cast<uint64_t>(irq13), 0x08, 0x8E);
    IDT::set_entry(&entries[46], reinterpret_cast<uint64_t>(irq14), 0x08, 0x8E);
    IDT::set_entry(&entries[47], reinterpret_cast<uint64_t>(irq15), 0x08, 0x8E);
    IDT::idt_load(reinterpret_cast<addr_t>(&descriptor));
}
}  // namespace IDT
