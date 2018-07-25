#pragma once

template <typename M, typename N>
struct Pair {
    Pair(M m, N n) : first(m), second(n){};

    M first;
    N second;
};

template <typename M, typename N>
Pair<M, N> make_pair(M m, N n)
{
    return Pair<M, N>(m, n);
}