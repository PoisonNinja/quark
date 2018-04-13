#pragma once

#include <kernel/time/clock.h>

namespace Time
{
class TSC : public Clock
{
public:
    TSC();
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
    time_t calibrated_frequency;
};
}  // namespace Time
