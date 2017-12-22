#pragma once

#include <kernel/time/clock.h>

namespace Time
{
class Intel8253 : public Clock
{
public:
    time_t read() override;
    time_t frequency() override;
    status_t enable() override;
    status_t disable() override;

private:
    time_t counter;
};
}  // namespace Time
