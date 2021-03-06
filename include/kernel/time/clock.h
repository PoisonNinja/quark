#pragma once

#include <lib/list.h>
#include <types.h>

namespace time
{
constexpr int feature_timer = 0x1;
constexpr int feature_clock = 0x2;
constexpr int feature_hr    = 0x4;

class clock
{
public:
    virtual int features()                 = 0;
    virtual time_t read()                  = 0;
    virtual time_t frequency()             = 0;
    virtual bool enable()                  = 0;
    virtual bool disable()                 = 0;
    virtual bool schedule(time_t interval) = 0;
    virtual bool periodic()                = 0;
    virtual const char* name()             = 0;
    libcxx::node<clock> node;
};
} // namespace time
