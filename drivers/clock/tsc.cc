#include <drivers/clock/tsc.h>
#include <kernel/time/time.h>

namespace Time
{
uint64_t TSC::rdtsc()
{
    unsigned int low, high;
    __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

TSC::TSC()
{
    // Automatic calibration
    calibrated_frequency = 0;
}

int TSC::features()
{
    return feature_clock;
}

time_t TSC::read()
{
    // Ticks emulated using variable since the PIT itself is useless
    return 0;
}

time_t TSC::frequency()
{
    return calibrated_frequency;
}

bool TSC::enable()
{
    return true;
}

bool TSC::disable()
{
    return true;
}

bool TSC::schedule(time_t /**/)
{
    return false;
}

bool TSC::periodic()
{
    return false;
}

const char* TSC::name()
{
    return "x86 TSC";
}
}  // namespace Time
