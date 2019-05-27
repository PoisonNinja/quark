#include <arch/cpu/feature.h>
#include <cpu/cpu.h>
#include <drivers/clock/tsc.h>
#include <kernel.h>
#include <kernel/time/time.h>

namespace time
{
uint64_t tsc::rdtsc()
{
    unsigned int low, high;
    __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

tsc::tsc()
{
    cpu::core* cpu = cpu::get_current_core();
    if (!cpu::x86_64::has_feature(*cpu, X86_FEATURE_CONSTANT_TSC)) {
        log::printk(
            log::log_level::WARNING,
            "tsc: CPU doesn't support constant tsc, timing will be unstable\n");
    } else {
        log::printk(log::log_level::INFO,
                    "tsc: CPU supports constant tsc :)\n");
    }
    // Automatic calibration
    log::printk(log::log_level::INFO, "tsc: Preparing to calibrate\n");
    calibrated_frequency = calibrate() * 1000;
    log::printk(log::log_level::INFO, "tsc: Calibrated to %llu kHZ\n",
                calibrated_frequency);
}

int tsc::features()
{
    return feature_clock;
}

time_t tsc::read()
{
    // Ticks emulated using variable since the PIT itself is useless
    return rdtsc();
}

time_t tsc::frequency()
{
    return calibrated_frequency;
}

bool tsc::enable()
{
    return true;
}

bool tsc::disable()
{
    return true;
}

bool tsc::schedule(time_t /**/)
{
    return false;
}

bool tsc::periodic()
{
    return false;
}

const char* tsc::name()
{
    return "x86 tsc";
}

uint64_t tsc::calibrate()
{
    time_t t1, t2;
    time_t nsec     = 10 * 1000 * 1000;
    uint64_t lowest = UINT64_MAX;
    for (int i = 0; i < 5; i++) {
        t1 = this->rdtsc();
        // Precalculated nsec to minimize overhead
        time::ndelay(nsec);
        t2            = this->rdtsc();
        uint64_t freq = (t2 - t1) / 10;
        log::printk(log::log_level::INFO, "tsc: Calibration round %d: %zu\n", i,
                    freq);
        if (freq < lowest) {
            lowest = freq;
        }
    }
    return lowest;
}
} // namespace time
