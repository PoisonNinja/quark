#include <cpu/interrupt.h>
#include <kernel/time/time.h>

#include <kernel.h>

namespace Time
{
static List<Timer, &Timer::node> timer_list;
static Timer* current_timer = NULL;

extern void arch_init();

void tick()
{
}

status_t register_timer(Timer& timer)
{
    timer_list.push_back(timer);
    if (current_timer) {
        if (timer.precision() > current_timer->precision()) {
            current_timer->disable();
            current_timer = &timer;
            timer.periodic();
        }
    } else {
        current_timer = &timer;
        timer.periodic();
    }
    return SUCCESS;
}

void init()
{
    Time::arch_init();
}
}  // namespace Time
