#include <drivers/clock/intel-8253.h>
#include <kernel/time/time.h>

namespace Time
{
void arch_init()
{
    Intel8253* pit = new Intel8253;
    register_timer(*pit);
}
}