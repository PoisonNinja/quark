#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/time/time.h>
#include <proc/sched.h>

namespace time
{
static libcxx::list<clock, &clock::node> clock_list;
static clock* current_ticker = nullptr;
static clock* current_clock  = nullptr;
static volatile struct timespec current_time;
static volatile time_t last = 0;

extern void arch_init();

void update_clock(clock* new_clock)
{
    current_clock = new_clock;
    last          = new_clock->read();
}

bool register_clock(clock& clock)
{
    clock_list.push_back(clock);
    if (clock.features() & feature_timer) {
        log::printk(log::log_level::INFO,
                    "Selecting %s as the system tick source\n", clock.name());
        if (current_ticker) {
            current_ticker->disable();
        }
        current_ticker = &clock;
        clock.periodic();
    }
    if (clock.features() & feature_clock) {
        if (!current_clock || current_clock->frequency() < clock.frequency()) {
            log::printk(log::log_level::INFO,
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
        log::printk(
            log::log_level::WARNING,
            "udelay received argument that is too large "
            "to safely pass to ndelay, unexpected behavior may occur\n");
    }
    ndelay(usec * 1000);
}

void mdelay(time_t msec)
{
    if (msec > UINT64_MAX / 1000 / 1000) {
        log::printk(
            log::log_level::WARNING,
            "mdelay received argument that is too large "
            "to safely pass to ndelay, unexpected behavior may occur\n");
    }
    ndelay(msec * 1000 * 1000);
}

void tick(struct interrupt_context* ctx)
{
    update();
    if (scheduler::online()) {
        scheduler::get_current_thread()->set_flag(thread_flag::RESCHEDULE,
                                                  true);
    }
}

void update()
{
    if (!current_clock) {
        return;
    }
    int flags;
    interrupt::save(flags);
    interrupt::disable();
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
    interrupt::restore(flags);
}

struct timespec now()
{
    time::update();
    time::timespec tm;
    tm.tv_sec  = current_time.tv_sec;
    tm.tv_nsec = current_time.tv_nsec;
    return tm;
}

void init()
{
    time::arch_init();
}
} // namespace time
