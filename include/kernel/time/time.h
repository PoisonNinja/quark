#pragma once

#include <kernel/time/clock.h>
#include <types.h>

namespace Time
{
class Clock;
class Timer;

struct timespec {
    time_t tv_sec;
    time_t tv_nsec;
};

void tick();
void init();
}  // namespace Time
