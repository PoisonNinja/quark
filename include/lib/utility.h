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
};

template <typename M, typename N>
libcxx::pair<M, N> make_pair(M m, N n)
{
    return libcxx::pair<M, N>(m, n);
}
} // namespace libcxx
