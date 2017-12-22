#include <arch/drivers/io.h>
#include <drivers/clock/intel-8253.h>

namespace Time
{
#define FREQUENCY 1000  // in HZ

time_t Intel8253::read()
{
    return counter;
}

time_t Intel8253::frequency()
{
    return FREQUENCY;
}

status_t Intel8253::enable()
{
}

status_t Intel8253::disable()
{
}
}  // namespace Time
