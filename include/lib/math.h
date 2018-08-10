#pragma once

#include <types.h>

namespace Math
{
template <typename T, typename U>
constexpr T round_up(T value, U interval)
{
    return ((value + interval - 1) / interval) * interval;
}

template <typename T, typename U>
constexpr T round_down(T value, U interval)
{
    return ((value / interval) * interval);
}

// Division, but rounding up
template <typename T, typename U>
constexpr T div_round_up(T x, U y)
{
    return (x + y - 1) / y;
}

constexpr size_t pow_2(unsigned int power)
{
    return 1 << power;
}

constexpr unsigned int log_2(unsigned long long number)
{
    return (8 * sizeof(unsigned long long) - __builtin_clzll(number) - 1);
}
}  // namespace Math