#include <drivers/clock/intel-8253.h>
#include <drivers/clock/tsc.h>
#include <kernel/time/time.h>

namespace time
{
void arch_init()
{
    intel8253* pit = new intel8253();
    register_clock(*pit);

    /*
     * This must be initialized after PIT since we depend on an existing tick
     * source to calibrate the TSC
     */
    tsc* t = new tsc();
    register_clock(*t);
}
} // namespace time
