#include <cpu/interrupt.h>
#include <kernel/time/time.h>

#include <kernel.h>

namespace Time
{
static List<Timer, &Timer::node> timer_list;
static Timer* current_timer = NULL;

extern void arch_init();

static void tick(int /*irq*/, void* /*dev_id*/, struct interrupt_ctx* /*ctx*/)
{
    Log::printk(Log::INFO, "Tick\n");
}

static struct Interrupt::Handler tick_handler = {
    .handler = tick,
    .dev_name = "tick",
    .dev_id = NULL,
};

status_t register_timer(Timer& timer)
{
    timer_list.push_back(timer);
    if (current_timer) {
        if (timer.precision() > current_timer->precision()) {
            Interrupt::unregister_handler(current_timer->irq(), tick_handler);
            current_timer->disable();
            current_timer = &timer;
            Interrupt::register_handler(timer.irq(), tick_handler);
            timer.periodic();
        }
    } else {
        current_timer = &timer;
        Interrupt::register_handler(timer.irq(), tick_handler);
        timer.periodic();
    }
    return SUCCESS;
}

void init()
{
    Time::arch_init();
}
}  // namespace Time
