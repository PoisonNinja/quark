#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/time/time.h>
#include <proc/sched.h>

namespace Time
{
static List<Timer, &Timer::node> timer_list;
static Timer* current_timer = nullptr;

extern void arch_init();

void tick(struct InterruptContext* ctx)
{
    Scheduler::tick(ctx);
}

status_t register_timer(Timer& timer)
{
    timer_list.push_back(timer);
    Log::printk(Log::INFO, "Selecting %s as the system tick source\n",
                timer.name());
    if (current_timer) {
        current_timer->disable();
    }
    current_timer = &timer;
    timer.periodic();
    return SUCCESS;
}

void init()
{
    Time::arch_init();
}
}  // namespace Time
