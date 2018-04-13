#pragma once

#include <kernel/time/clock.h>
#include <types.h>

namespace Time
{
class Clock;

struct timespec {
    time_t tv_sec;
    time_t tv_nsec;
};

void tick(struct InterruptContext* ctx);
void init();
}  // namespace Time
