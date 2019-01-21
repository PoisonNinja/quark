#pragma once

#include <kernel/time/clock.h>

namespace time
{
class tsc : public clock
{
public:
    tsc();
    int features() override;
    time_t read() override;
    time_t frequency() override;
    bool enable() override;
    bool disable() override;
    bool schedule(time_t interval) override;
    bool periodic() override;
    const char* name() override;

    static uint64_t rdtsc();

private:
    uint64_t calibrate();
    time_t calibrated_frequency;
};
} // namespace time
