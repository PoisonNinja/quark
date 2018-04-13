#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/time/time.h>
#include <proc/sched.h>

namespace Time
{
static List<Clock, &Clock::node> clock_list;
static Clock* current_ticker = nullptr;
static Clock* current_clock = nullptr;
static struct timespec current_time;
static time_t last;

extern void arch_init();

void update_clock(Clock* new_clock)
{
    current_clock = new_clock;
    last = new_clock->read();
}

bool register_clock(Clock& clock)
{
    clock_list.push_back(clock);
    if (clock.features() & feature_timer) {
        Log::printk(Log::INFO, "Selecting %s as the system tick source\n",
                    clock.name());
        if (current_ticker) {
            current_ticker->disable();
        }
        current_ticker = &clock;
        clock.periodic();
    }
    if (clock.features() & feature_clock) {
        if (!current_clock || current_clock->frequency() < clock.frequency()) {
            Log::printk(Log::INFO, "Selecting %s as the system clock\n",
                        clock.name());
            update_clock(&clock);
        }
    }
    return true;
}

void tick(struct InterruptContext* ctx)
{
    update();
    Scheduler::switch_next(ctx);
}

void update()
{
    if (!current_clock) {
        return;
    }
    time_t current = current_clock->read();
    time_t offset = current - last;
    last = current;
    time_t nsec = offset * 1000000000 / current_clock->frequency();
    current_time.tv_nsec += nsec;
    // Coalesce nanoseconds into seconds
    while (current_time.tv_nsec >= 1000000000) {
        current_time.tv_nsec -= 1000000000;
        current_time.tv_sec++;
    }
}

struct timespec now()
{
    Time::update();
    return current_time;
}

void init()
{
    Time::arch_init();
}
}  // namespace Time
