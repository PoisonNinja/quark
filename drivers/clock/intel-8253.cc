#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <drivers/clock/intel-8253.h>
#include <kernel/time/time.h>

namespace Time
{
const int FREQUENCY = 1193181;  // Frequency in HZ
const int HZ = 1000;            // # of interrupts per second
const int CHANNEL0 = 0x40;
// const int CHANNEL1 = 0x41;
// const int CHANNEL2 = 0x42;
const int COMMAND = 0x43;
const char* NAME = "Intel 8253";

static void interrupt_handler(int /* irq */, void* /* dev_id */,
                              struct InterruptContext* ctx)
{
    Time::tick(ctx);
}

static struct Interrupt::Handler handler(interrupt_handler, NAME, &handler);

status_t Intel8253::schedule(time_t interval)
{
    outb(COMMAND, 0x38);  // Channel 0, lobyte/hibyte, mode 4, 16-bit
    outb(CHANNEL0, static_cast<uint8_t>(interval & 0xFF));
    outb(CHANNEL0, static_cast<uint8_t>((interval >> 8) & 0xFF));
    return SUCCESS;
}

status_t Intel8253::periodic()
{
    register_handler(Interrupt::irq_to_interrupt(0), handler);
    uint16_t divisor = FREQUENCY / HZ;
    outb(COMMAND, 0x36);  // Channel 0, lobyte/hibyte, mode 2, 16-bit
    outb(CHANNEL0, static_cast<uint8_t>(divisor & 0xFF));
    outb(CHANNEL0, static_cast<uint8_t>((divisor >> 8) & 0xFF));
    return SUCCESS;
}

status_t Intel8253::disable()
{
    outb(COMMAND, 0x30);
    outb(CHANNEL0, 0);
    outb(CHANNEL0, 0);
    unregister_handler(Interrupt::irq_to_interrupt(0), handler);
    return SUCCESS;
}

const char* Intel8253::name()
{
    return NAME;
}
}  // namespace Time
