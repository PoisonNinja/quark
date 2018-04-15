#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <drivers/clock/intel-8253.h>
#include <kernel/time/time.h>

namespace Time
{
constexpr int FREQUENCY = 1193181;  // Frequency in HZ
constexpr int HZ = 1000;            // # of interrupts per second
constexpr int CHANNEL0 = 0x40;
// const int CHANNEL1 = 0x41;
// const int CHANNEL2 = 0x42;
constexpr int COMMAND = 0x43;
const char* NAME = "Intel 8253";

static time_t ticks = 0;

static void interrupt_handler(int /* irq */, void* /*clock*/,
                              struct InterruptContext* ctx)
{
    ticks++;
    Time::tick(ctx);
}

static struct Interrupt::Handler handler(interrupt_handler, NAME, &handler);

int Intel8253::features()
{
    return feature_clock | feature_timer;
}

time_t Intel8253::read()
{
    // Ticks emulated using variable since the PIT itself is useless
    return ticks;
}

time_t Intel8253::frequency()
{
    return HZ;
}

bool Intel8253::enable()
{
    return true;
}

bool Intel8253::disable()
{
    outb(COMMAND, 0x30);
    outb(CHANNEL0, 0);
    outb(CHANNEL0, 0);
    unregister_handler(Interrupt::irq_to_interrupt(0), handler);
    return true;
}

bool Intel8253::schedule(time_t interval)
{
    outb(COMMAND, 0x38);  // Channel 0, lobyte/hibyte, mode 4, 16-bit
    outb(CHANNEL0, static_cast<uint8_t>(interval & 0xFF));
    outb(CHANNEL0, static_cast<uint8_t>((interval >> 8) & 0xFF));
    return true;
}

bool Intel8253::periodic()
{
    register_handler(Interrupt::irq_to_interrupt(0), handler);
    uint16_t divisor = FREQUENCY / HZ;
    outb(COMMAND, 0x36);  // Channel 0, lobyte/hibyte, mode 2, 16-bit
    outb(CHANNEL0, static_cast<uint8_t>(divisor & 0xFF));
    outb(CHANNEL0, static_cast<uint8_t>((divisor >> 8) & 0xFF));
    return true;
}

const char* Intel8253::name()
{
    return NAME;
}
}  // namespace Time
