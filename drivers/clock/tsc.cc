#include <arch/cpu/feature.h>
#include <cpu/cpu.h>
#include <drivers/clock/tsc.h>
#include <kernel.h>
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
    CPU::Core* cpu = CPU::get_current_core();
    if (!CPU::X64::has_feature(*cpu, X86_FEATURE_CONSTANT_TSC)) {
        Log::printk(
            Log::WARNING,
            "tsc: CPU doesn't support constant TSC, timing will be unstable\n");
    }
    // Automatic calibration
    Log::printk(Log::INFO, "tsc: Preparing to calibrate\n");
    calibrated_frequency = calibrate() * 1000;
    Log::printk(Log::INFO, "tsc: Calibrated to %llu kHZ\n",
                calibrated_frequency);
}

int TSC::features()
{
    return feature_clock;
}

time_t TSC::read()
{
    // Ticks emulated using variable since the PIT itself is useless
    return rdtsc();
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

uint64_t TSC::calibrate()
{
    time_t t1, t2;
    time_t nsec = 10 * 1000 * 1000;
    uint64_t lowest = UINT64_MAX;
    for (int i = 0; i < 5; i++) {
        t1 = this->rdtsc();
        // Precalculated nsec to minimize overhead
        Time::ndelay(nsec);
        t2 = this->rdtsc();
        uint64_t freq = (t2 - t1) / 10;
        if (freq < lowest) {
            lowest = freq;
        }
    }
    return lowest;
}
}  // namespace Time
