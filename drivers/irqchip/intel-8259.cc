#include <arch/drivers/io.h>
#include <drivers/irqchip/intel-8259.h>

namespace irqchip
{
static const int PIC1_COMMAND = 0x20;
static const int PIC1_DATA    = 0x21;
static const int PIC2_COMMAND = 0xA0;
static const int PIC2_DATA    = 0xA1;

static const int PIC1_OFFSET = 0x20; // 32
static const int PIC2_OFFSET = 0x28; // 40

static const int COMMAND_EOI  = 0x20;
static const int COMMAND_INIT = 0x10; /* Initialization - required! */

static const int CONFIG_ICW4 = 0x01; /* ICW4 (not) needed */
static const int CONFIG_8086 = 0x01; /* 8086/88 (MCS-80/85) mode */

bool intel8259::enable()
{
    outb(PIC1_COMMAND, COMMAND_INIT + CONFIG_ICW4);
    iowait();
    outb(PIC2_COMMAND, COMMAND_INIT + CONFIG_ICW4);
    iowait();
    outb(PIC1_DATA, PIC1_OFFSET);
    iowait();
    outb(PIC2_DATA, PIC2_OFFSET);
    iowait();
    outb(PIC1_DATA, 4);
    iowait();
    outb(PIC2_DATA, 2);
    iowait();

    outb(PIC1_DATA, CONFIG_8086);
    iowait();
    outb(PIC2_DATA, CONFIG_8086);
    iowait();

    return true;
}

bool intel8259::disable()
{
    for (int i = 0; i < 16; i++) {
        intel8259::mask(i);
    }
    return true;
}

bool intel8259::mask(uint32_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);
    return true;
}

bool intel8259::unmask(uint32_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);
    return true;
}

bool intel8259::ack(uint32_t irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, COMMAND_EOI);
    }
    outb(PIC1_COMMAND, COMMAND_EOI);
    return true;
}

bool intel8259::spurious()
{
    // TODO: Implement
    return false;
}
} // namespace irqchip
