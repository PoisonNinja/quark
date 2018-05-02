#include <drivers/clock/intel-8253.h>
#include <drivers/clock/tsc.h>
#include <kernel/time/time.h>

namespace Time
{
void arch_init()
{
    Intel8253* pit = new Intel8253();
    register_clock(*pit);

    // // This must be initialized after TSC
    // TSC* tsc = new TSC();
    // register_clock(*tsc);
}
}