#pragma once

namespace libcxx
{
template <typename M, typename N>
struct pair {
    pair(M m, N n)
        : first(m)
        , second(n){};

    M first;
    N second;

    pair& operator=(const pair& other)
    {
        this->first  = other.first;
        this->second = other.second;
        return *this;
    }

    constexpr bool operator==(const pair<M, N>& rhs)
    {
        return ((this->first == rhs.first) && (this->second == rhs.second));
    }

    constexpr bool operator!=(const pair<M, N>& rhs)
    {
        return !(*this == rhs);
    }
};

template <typename M, typename N>
libcxx::pair<M, N> make_pair(M m, N n)
{
    return libcxx::pair<M, N>(m, n);
}
} // namespace libcxx
