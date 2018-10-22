#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/time/time.h>
#include <proc/sched.h>

namespace Time
{
static List<Clock, &Clock::node> clock_list;
static Clock* current_ticker = nullptr;
static Clock* current_clock  = nullptr;
static volatile struct timespec current_time;
static volatile time_t last = 0;

extern void arch_init();

void update_clock(Clock* new_clock)
{
    current_clock = new_clock;
    last          = new_clock->read();
}

bool register_clock(Clock& clock)
{
    clock_list.push_back(clock);
    if (clock.features() & feature_timer) {
        Log::printk(Log::LogLevel::INFO,
                    "Selecting %s as the system tick source\n", clock.name());
        if (current_ticker) {
            current_ticker->disable();
        }
        current_ticker = &clock;
        clock.periodic();
    }
    if (clock.features() & feature_clock) {
        if (!current_clock || current_clock->frequency() < clock.frequency()) {
            Log::printk(Log::LogLevel::INFO,
                        "Selecting %s as the system clock\n", clock.name());
            update_clock(&clock);
        }
    }
    return true;
}

void ndelay(time_t nsec)
{
    update();
    time_t current = current_time.tv_nsec + current_time.tv_sec * nsec_per_sec;
    time_t target  = current + nsec;
    while (current < target) {
        current = current_time.tv_nsec + current_time.tv_sec * nsec_per_sec;
    }
}

void udelay(time_t usec)
{
    if (usec > UINT64_MAX / 1000) {
        Log::printk(
            Log::LogLevel::WARNING,
            "udelay received argument that is too large "
            "to safely pass to ndelay, unexpected behavior may occur\n");
    }
    ndelay(usec * 1000);
}

void mdelay(time_t msec)
{
    if (msec > UINT64_MAX / 1000 / 1000) {
        Log::printk(
            Log::LogLevel::WARNING,
            "mdelay received argument that is too large "
            "to safely pass to ndelay, unexpected behavior may occur\n");
    }
    ndelay(msec * 1000 * 1000);
}

void tick(struct InterruptContext* ctx)
{
    update();
    if (Scheduler::online()) {
        Scheduler::switch_next(ctx);
    }
}

void update()
{
    if (!current_clock) {
        return;
    }
    int flags;
    Interrupt::save(flags);
    Interrupt::disable();
    time_t current = current_clock->read();
    time_t offset  = current - last;
    time_t nsec    = offset * nsec_per_sec / current_clock->frequency();
    current_time.tv_nsec += nsec;
    // Coalesce nanoseconds into seconds
    while (current_time.tv_nsec >= nsec_per_sec) {
        current_time.tv_nsec -= nsec_per_sec;
        current_time.tv_sec++;
    }
    last = current;
    Interrupt::restore(flags);
}

struct timespec now()
{
    Time::update();
    Time::timespec tm;
    tm.tv_sec  = current_time.tv_sec;
    tm.tv_nsec = current_time.tv_nsec;
    return tm;
}

void init()
{
    Time::arch_init();
}
} // namespace Time
