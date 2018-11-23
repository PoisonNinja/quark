#pragma once

#include <lib/memory.h>
#include <lib/utility.h>
#include <type_traits>
#include <types.h>

namespace libcxx
{
template <class Key>
struct hash {
    size_t operator()(const Key k) const;
};

/*
 * Pointer specialization
 *
 * Uses the value of the pointer as the key
 */
template <class Key>
struct hash<Key *> {
    size_t operator()(const Key *k) const
    {
        return reinterpret_cast<size_t>(k);
    }
};

/*
 * Integer specialization
 */
template <>
struct hash<bool> {
    size_t operator()(const bool k) const
    {
        return static_cast<size_t>(k);
    }
};

template <>
struct hash<unsigned int> {
    size_t operator()(const unsigned int k) const
    {
        return static_cast<size_t>(k);
    }
};

template <>
struct hash<unsigned long> {
    size_t operator()(const unsigned long k) const
    {
        return static_cast<size_t>(k);
    }
};

template <>
struct hash<unsigned long long> {
    size_t operator()(const unsigned long long k) const
    {
        return static_cast<size_t>(k);
    }
};

template <class T>
class reference_wrapper;

namespace detail
{
template <class T>
struct is_reference_wrapper : libcxx::false_type {
};
template <class U>
struct is_reference_wrapper<libcxx::reference_wrapper<U>> : libcxx::true_type {
};
template <class T>
constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

template <class T, class Type, class T1, class... Args>
decltype(auto) INVOKE(Type T::*f, T1 &&t1, Args &&... args)
{
    if constexpr (libcxx::is_member_function_pointer_v<decltype(f)>) {
        if constexpr (libcxx::is_base_of_v<T, libcxx::decay_t<T1>>)
            return (libcxx::forward<T1>(t1).*f)(libcxx::forward<Args>(args)...);
        else if constexpr (is_reference_wrapper_v<libcxx::decay_t<T1>>)
            return (t1.get().*f)(libcxx::forward<Args>(args)...);
        else
            return ((*libcxx::forward<T1>(t1)).*
                    f)(libcxx::forward<Args>(args)...);
    } else {
        static_assert(libcxx::is_member_object_pointer_v<decltype(f)>);
        static_assert(sizeof...(args) == 0);
        if constexpr (libcxx::is_base_of_v<T, libcxx::decay_t<T1>>)
            return libcxx::forward<T1>(t1).*f;
        else if constexpr (is_reference_wrapper_v<libcxx::decay_t<T1>>)
            return t1.get().*f;
        else
            return (*libcxx::forward<T1>(t1)).*f;
    }
}

template <class F, class... Args>
decltype(auto) INVOKE(F &&f, Args &&... args)
{
    return libcxx::forward<F>(f)(libcxx::forward<Args>(args)...);
}

template <typename AlwaysVoid, typename, typename...>
struct invoke_result {
};
template <typename F, typename... Args>
struct invoke_result<decltype(void(detail::INVOKE(std::declval<F>(),
                                                  std::declval<Args>()...))),
                     F, Args...> {
    using type =
        decltype(detail::INVOKE(std::declval<F>(), std::declval<Args>()...));
};
} // namespace detail

template <class>
struct result_of;
template <class F, class... ArgTypes>
struct result_of<F(ArgTypes...)> : detail::invoke_result<void, F, ArgTypes...> {
};

template <class T>
using result_of_t = typename result_of<T>::type;

template <class F, class... ArgTypes>
struct invoke_result : detail::invoke_result<void, F, ArgTypes...> {
};

template <class F, class... ArgTypes>
using invoke_result_t = typename invoke_result<F, ArgTypes...>::type;

template <class F, class... Args>
libcxx::invoke_result_t<F, Args...> invoke(F &&f, Args &&... args)
{
    return detail::INVOKE(libcxx::forward<F>(f),
                          libcxx::forward<Args>(args)...);
}

template <class T>
class reference_wrapper
{
public:
    // types
    typedef T type;

    // construct/copy/destroy
    reference_wrapper(T &ref) noexcept
        : _ptr(libcxx::addressof(ref))
    {
    }
    reference_wrapper(T &&)                               = delete;
    reference_wrapper(const reference_wrapper &) noexcept = default;

    // assignment
    reference_wrapper &operator=(const reference_wrapper &x) noexcept = default;

    // access
    operator T &() const noexcept
    {
        return *_ptr;
    }
    T &get() const noexcept
    {
        return *_ptr;
    }

    template <class... ArgTypes>
    libcxx::invoke_result_t<T &, ArgTypes...>
    operator()(ArgTypes &&... args) const
    {
        return libcxx::invoke(get(), libcxx::forward<ArgTypes>(args)...);
    }

private:
    T *_ptr;
};

// deduction guides
template <class T>
reference_wrapper(reference_wrapper<T>)->reference_wrapper<T>;

/*
 * Taken from
 * https://stackoverflow.com/questions/14739902/is-there-a-standalone-implementation-of-stdfunction
 */
template <typename Result, typename... Args>
struct abstract_function {
    virtual Result operator()(Args... args)  = 0;
    virtual abstract_function *clone() const = 0;
    virtual ~abstract_function()             = default;
};

template <typename Func, typename Result, typename... Args>
class concrete_function : public abstract_function<Result, Args...>
{
    Func f;

public:
    concrete_function(const Func &x)
        : f(x)
    {
    }
    Result operator()(Args... args) override
    {
        return f(args...);
    }
    concrete_function *clone() const override
    {
        return new concrete_function{f};
    }
};

template <typename Func>
struct func_filter {
    typedef Func type;
};
template <typename Result, typename... Args>
struct func_filter<Result(Args...)> {
    typedef Result (*type)(Args...);
};

template <typename signature>
class function;

template <typename Result, typename... Args>
class function<Result(Args...)>
{
    abstract_function<Result, Args...> *f;

public:
    function()
        : f(nullptr)
    {
    }
    template <typename Func>
    function(const Func &x)
        : f(new concrete_function<typename func_filter<Func>::type, Result,
                                  Args...>(x))
    {
    }
    function(const function &rhs)
        : f(rhs.f ? rhs.f->clone() : nullptr)
    {
    }
    function &operator=(const function &rhs)
    {
        if ((&rhs != this) && (rhs.f)) {
            auto *temp = rhs.f->clone();
            delete f;
            f = temp;
        }
        return *this;
    }
    template <typename Func>
    function &operator=(const Func &x)
    {
        auto *temp = new concrete_function<typename func_filter<Func>::type,
                                           Result, Args...>(x);
        delete f;
        f = temp;
        return *this;
    }
    Result operator()(Args... args)
    {
        if (f)
            return (*f)(args...);
        else
            return Result();
    }
    ~function()
    {
        delete f;
    }
};

/*
 * From libcxx
 */
template <class _Tp>
struct __is_placeholder : public integral_constant<int, 0> {
};

template <class _Tp>
struct is_placeholder : public __is_placeholder<typename remove_cv<_Tp>::type> {
};

namespace placeholders
{
template <int _Np>
struct __ph {
};

/* _LIBCPP_INLINE_VAR */ constexpr __ph<1> _1{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<2> _2{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<3> _3{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<4> _4{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<5> _5{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<6> _6{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<7> _7{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<8> _8{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<9> _9{};
/* _LIBCPP_INLINE_VAR */ constexpr __ph<10> _10{};

} // namespace placeholders

template <int _Np>
struct __is_placeholder<placeholders::__ph<_Np>>
    : public integral_constant<int, _Np> {
};

/*
 * Take from https://gist.github.com/Redchards/c5be14c2998f1ca1d757
 */

// Actually, a subtle bug may arise when the function is using index_constant as
// parameter. For this reason, a more correct implementation should define a
// specialized class, hidden in a namespace, and forbid the use in other places.
// An easier, more correct, but more verbose way would be to just define two
// different classes for the argument list and the bounded arguments, to avoid
// the confusion with the index_constant bracket operator overload. However, as
// this is only an academic example, it should strive to stay extremly simple
// and straightforward, so this improvment will not be applied. I just wanted
// anyone wanting to use this somewhere to be aware of this caveheat. Moreover,
// you should use libcxx::bind anyway.

template <size_t n>
using index_constant = libcxx::integral_constant<size_t, n>;

template <class... Args>
class binder_list
{
public:
    template <class... TArgs>
    constexpr binder_list(TArgs &&... args) noexcept
        : boundedArgs_{libcxx::forward<TArgs>(args)...}
    {
    }

    template <size_t n>
    constexpr decltype(auto) operator[](index_constant<n>) noexcept
    {
        return libcxx::get<n>(boundedArgs_);
    }

private:
    libcxx::tuple<Args...> boundedArgs_;
};

template <class... Args>
class callee_list
{
public:
    template <class... TArgs>
    constexpr callee_list(TArgs &&... args) noexcept
        : boundedArgs_{libcxx::forward<TArgs>(args)...}
    {
    }

    template <class T,
              libcxx::enable_if_t<(libcxx::is_placeholder<
                                       libcxx::remove_reference_t<T>>::value ==
                                   0)> * = nullptr>
    constexpr decltype(auto) operator[](T &&t)
    {
        return libcxx::forward<T>(t);
    }

    template <class T, libcxx::enable_if_t<(libcxx::is_placeholder<T>::value !=
                                            0)> * = nullptr>
    constexpr decltype(auto) operator[](T)
    {
        return libcxx::get<libcxx::is_placeholder<T>::value - 1>(
            libcxx::move(boundedArgs_));
    }

private:
    libcxx::tuple<Args &&...> boundedArgs_;
};

template <class Fn, class... Args>
class binder
{
public:
    template <class TFn, class... TArgs>
    constexpr binder(TFn &&f, TArgs &&... args) noexcept
        : f_{libcxx::forward<TFn>(f)}
        , argumentList_{libcxx::forward<TArgs>(args)...}
    {
    }

    // Please C++, give me a way of detecting noexcept :'(
    template <class... CallArgs>
    constexpr decltype(auto) operator()(CallArgs &&... args)
    // noexcept(noexcept(call(libcxx::make_index_sequence<sizeof...(Args)>{},
    // libcxx::declval<Args>()...)))
    {
        return call(libcxx::make_index_sequence<sizeof...(Args)>{},
                    libcxx::forward<CallArgs>(args)...);
    }

private:
    template <class... CallArgs, size_t... Seq>
    constexpr decltype(auto) call(libcxx::index_sequence<Seq...>,
                                  CallArgs &&... args)
    // noexcept(noexcept(f_(this->binder_list<CallArgs...>{libcxx::declval<CallArgs>()...}[this->argumentList_[index_constant<Seq>{}]]...)))
    {
        return libcxx::invoke(
            f_, (callee_list<CallArgs...>{libcxx::forward<CallArgs>(
                    args)...}[argumentList_[index_constant<Seq>{}]])...);
    }

private:
    Fn f_;
    binder_list<Args...> argumentList_;
};

template <class Fn, class... Args>
binder<Fn, Args...> bind(Fn &&f, Args &&... args)
{
    return binder<Fn, Args...>{libcxx::forward<Fn>(f),
                               libcxx::forward<Args>(args)...};
}
} // namespace libcxx
