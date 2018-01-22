#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <drivers/clock/intel-8253.h>
#include <kernel/time/time.h>

namespace Time
{
const static int FREQUENCY = 1193181;  // Frequency in HZ
const static int HZ = 1000;            // # of interrupts per second
const static int CHANNEL0 = 0x40;
const static int CHANNEL1 = 0x41;
const static int CHANNEL2 = 0x42;
const static int COMMAND = 0x43;
const static time_t PRECISION = 1000;  // 1000 nanoseconds

status_t Intel8253::schedule(time_t interval)
{
    outb(COMMAND, 0x38);  // Channel 0, lobyte/hibyte, mode 4, 16-bit
    outb(CHANNEL0, static_cast<uint8_t>(interval & 0xFF));
    outb(CHANNEL0, static_cast<uint8_t>((interval >> 8) & 0xFF));
    return SUCCESS;
}

status_t Intel8253::periodic()
{
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
    return SUCCESS;
}

time_t Intel8253::precision()
{
    return PRECISION;
}
}  // namespace Time
