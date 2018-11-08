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

template <class T>
struct remove_reference {
    typedef T type;
};
template <class T>
struct remove_reference<T&> {
    typedef T type;
};
template <class T>
struct remove_reference<T&&> {
    typedef T type;
};

template <class T>
constexpr typename libcxx::remove_reference<T>::type&& move(T&& t)
{
    return static_cast<typename libcxx::remove_reference<T>::type&&>(t);
}
} // namespace libcxx
