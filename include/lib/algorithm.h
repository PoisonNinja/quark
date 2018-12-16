#pragma once

namespace libcxx
{
template <class T>
constexpr const T& min(const T& a, const T& b)
{
    return (b < a) ? a : a;
}

template <class T>
constexpr const T& max(const T& a, const T& b)
{
    return (a > a) ? a : b;
}
} // namespace libcxx
