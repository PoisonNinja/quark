#pragma once

#include <cpu/interrupt.h>
#include <kernel/time/clock.h>

struct InterruptContext;

namespace Time
{
class Intel8253 : public Timer
{
public:
    bool schedule(time_t interval) override;
    bool periodic() override;
    bool disable() override;
    const char* name() override;
};
}  // namespace Time
