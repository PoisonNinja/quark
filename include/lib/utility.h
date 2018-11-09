#pragma once

namespace libcxx
{
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

template <class T>
constexpr void swap(T& a, T& b)
{
    T t = libcxx::move(a);
    a   = libcxx::move(b);
    b   = libcxx::move(t);
}

template <typename M, typename N>
struct pair {
    constexpr pair()
    {
    }

    constexpr pair(M m, N n)
        : first(m)
        , second(n){};

    pair(const pair& p) = default;

    constexpr pair(pair&& p) = default;

    M first;
    N second;

    pair& operator=(const pair& other)
    {
        this->first  = other.first;
        this->second = other.second;
        return *this;
    }

    pair& operator=(const pair&& other)
    {
        libcxx::swap(this->first, other.first);
        libcxx::swap(this->second, other.second);
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
