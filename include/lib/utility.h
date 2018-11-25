#pragma once

#include <lib/type_traits.h>

namespace libcxx
{
template <class T>
constexpr typename libcxx::remove_reference<T>::type&& move(T&& t)
{
    return static_cast<typename libcxx::remove_reference<T>::type&&>(t);
}

template <class T>
constexpr T&& forward(typename libcxx::remove_reference<T>::type& t) noexcept
{
    return static_cast<T&&>(t);
}

template <class T>
constexpr T&& forward(typename libcxx::remove_reference<T>::type&& t) noexcept
{
    return static_cast<T&&>(t);
};

template <class T>
constexpr void swap(T& a, T& b)
{
    T t = libcxx::move(a);
    a   = libcxx::move(b);
    b   = libcxx::move(t);
}

template <class... Ts>
struct tuple;

template <class T, class... Ts>
struct tuple<T, Ts...> {
    T first;
    tuple<Ts...> rest;

    tuple() = default;
    template <class U, class... Us,
              class = typename ::libcxx::enable_if<!::libcxx::is_base_of<
                  tuple, typename ::libcxx::decay<U>::type>::value>::type>
    tuple(U&& u, Us&&... tail)
        : first(::libcxx::forward<U>(u))
        , rest(::libcxx::forward<Us>(tail)...)
    {
    }
};

template <class T>
struct tuple<T> {
    T first;

    tuple() = default;
    template <class U,
              class = typename ::libcxx::enable_if<!::libcxx::is_base_of<
                  tuple, typename ::libcxx::decay<U>::type>::value>::type>
    tuple(U&& u)
        : first(::libcxx::forward<U>(u))
    {
    }
};

template <>
struct tuple<> {
};

namespace detail
{

template <::size_t i, class T>
struct tuple_element;

template <::size_t i, class T, class... Ts>
struct tuple_element<i, tuple<T, Ts...>> : tuple_element<i - 1, tuple<Ts...>> {
};

template <class T, class... Ts>
struct tuple_element<0, tuple<T, Ts...>> {
    using type = T;
};

template <::size_t i>
struct tuple_accessor {
    template <class... Ts>
    static inline typename tuple_element<i, tuple<Ts...>>::type&
    get(tuple<Ts...>& t)
    {
        return tuple_accessor<i - 1>::get(t.rest);
    }

    template <class... Ts>
    static inline const typename tuple_element<i, tuple<Ts...>>::type&
    get(const tuple<Ts...>& t)
    {
        return tuple_accessor<i - 1>::get(t.rest);
    }
};

template <>
struct tuple_accessor<0> {
    template <class... Ts>
    static inline typename tuple_element<0, tuple<Ts...>>::type&
    get(tuple<Ts...>& t)
    {
        return t.first;
    }

    template <class... Ts>
    static inline const typename tuple_element<0, tuple<Ts...>>::type&
    get(const tuple<Ts...>& t)
    {
        return t.first;
    }
};

} // namespace detail

template <class... Ts>
inline tuple<typename ::libcxx::decay<Ts>::type...> make_tuple(Ts&&... x)
{
    return tuple<typename ::libcxx::decay<Ts>::type...>(
        ::libcxx::forward<Ts>(x)...);
}

template <::size_t i, class... Ts>
inline typename detail::tuple_element<i, tuple<Ts...>>::type&
get(tuple<Ts...>& t)
{
    return detail::tuple_accessor<i>::get(t);
}

template <::size_t i, class... Ts>
inline const typename detail::tuple_element<i, tuple<Ts...>>::type&
get(const tuple<Ts...>& t)
{
    return detail::tuple_accessor<i>::get(t);
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

template <class _Arg, class _Result>
struct unary_function {
    typedef _Arg argument_type;
    typedef _Result result_type;
};

template <class T, class U = T>
T exchange(T& obj, U&& new_value)
{
    T old_value = libcxx::move(obj);
    obj         = libcxx::forward<U>(new_value);
    return old_value;
}
} // namespace libcxx
