#pragma once

#include <kernel/time/clock.h>
#include <types.h>

namespace time
{
constexpr time_t nsec_per_sec = 1000000000;

class clock;

struct timespec {
    time_t tv_sec;
    time_t tv_nsec;
};

void update_clock(clock* new_clock);
bool register_clock(clock& clock);

void ndelay(time_t nsecs);
void ndelay(time_t usecs);
void mdelay(time_t msecs);

struct timespec now();

void update();
void tick(struct interrupt_context* ctx);
void init();
} // namespace time
