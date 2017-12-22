#pragma once

#include <types.h>

namespace Time
{
class Clock;

struct timespec {
    time_t tv_sec;
    time_t tv_nsec;
};

void init();
}  // namespace Time
