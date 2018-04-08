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
    virtual bool enable() = 0;
    virtual bool disable() = 0;
    Node<Clock> node;
};

class Timer
{
public:
    virtual bool schedule(time_t interval) = 0;
    virtual bool periodic() = 0;
    virtual bool disable() = 0;
    virtual const char* name() = 0;
    Node<Timer> node;
};

bool register_timer(Timer& timer);
}  // namespace Time
