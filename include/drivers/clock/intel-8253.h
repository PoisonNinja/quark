#pragma once

#include <kernel/time/clock.h>

struct interrupt_ctx;

namespace Time
{
class Intel8253 : public Timer
{
public:
    status_t schedule(time_t interval) override;
    status_t periodic() override;
    status_t disable() override;
    time_t precision() override;
    int irq() override;
};
}  // namespace Time
