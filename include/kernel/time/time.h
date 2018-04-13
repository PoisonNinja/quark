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

void update();
struct timespec now();
void tick(struct InterruptContext* ctx);
void init();
}  // namespace Time
