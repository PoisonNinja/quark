#pragma once

#include <cpu/interrupt.h>
#include <kernel/time/clock.h>

struct InterruptContext;

namespace Time
{
class Intel8253 : public Clock
{
public:
    Intel8253(){};
    time_t read() override;
    time_t frequency() override;
    bool enable() override;
    bool disable() override;
    bool schedule(time_t interval) override;
    bool periodic() override;
    const char* name() override;
};
}  // namespace Time
