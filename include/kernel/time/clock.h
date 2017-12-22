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
}  // namespace Time
