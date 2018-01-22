#pragma once

#include <lib/list.h>
#include <types.h>

namespace Time
{
class Clock
{
public:
    virtual time_t read() = 0;
    virtual time_t frequency() = 0;
    virtual status_t enable() = 0;
    virtual status_t disable() = 0;
    Node<Clock> node;
};

class Timer
{
public:
    virtual status_t schedule(time_t interval) = 0;
    virtual status_t periodic() = 0;
    virtual status_t disable() = 0;
    virtual time_t precision() = 0;
    Node<Timer> node;
};

status_t register_timer(Timer& timer);
}  // namespace Time
