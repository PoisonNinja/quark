#pragma once

#include <kernel/time/clock.h>
#include <types.h>

namespace Time
{
constexpr time_t nsec_per_sec = 1000000000;

class Clock;

struct timespec {
    time_t tv_sec;
    time_t tv_nsec;
};

void update_clock(Clock* new_clock);
bool register_clock(Clock& clock);

void ndelay(time_t nsecs);
void ndelay(time_t usecs);
void mdelay(time_t msecs);

struct timespec now();

void update();
void tick(struct InterruptContext* ctx);
void init();
}  // namespace Time
