#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/time/time.h>
#include <proc/sched.h>

namespace Time
{
static List<Clock, &Clock::node> clock_list;
static Clock* current_ticker = nullptr;

extern void arch_init();

void tick(struct InterruptContext* ctx)
{
    Scheduler::switch_next(ctx);
}

bool register_clock(Clock& clock)
{
    clock_list.push_back(clock);
    Log::printk(Log::INFO, "Selecting %s as the system tick source\n",
                clock.name());
    if (current_ticker) {
        current_ticker->disable();
    }
    current_ticker = &clock;
    clock.periodic();
    return true;
}

void init()
{
    Time::arch_init();
}
}  // namespace Time
