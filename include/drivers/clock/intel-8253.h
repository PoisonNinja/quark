#pragma once

#include <cpu/interrupt.h>
#include <kernel/time/clock.h>

struct interrupt_context;

namespace time
{
class intel8253 : public clock
{
public:
    intel8253(){};
    int features() override;
    time_t read() override;
    time_t frequency() override;
    bool enable() override;
    bool disable() override;
    bool schedule(time_t interval) override;
    bool periodic() override;
    const char* name() override;
};
} // namespace time
