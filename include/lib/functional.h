#pragma once

#include <lib/memory.h>
#include <lib/new.h>
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
struct invoke_result<decltype(void(detail::INVOKE(libcxx::declval<F>(),
                                                  libcxx::declval<Args>()...))),
                     F, Args...> {
    using type = decltype(
        detail::INVOKE(libcxx::declval<F>(), libcxx::declval<Args>()...));
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
 * Based off of SG14's function
 *
 * https://github.com/WG21-SG14/SG14
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
namespace function_detail
{
static constexpr size_t InplaceFunctionDefaultCapacity = 32;

#if defined(__GLIBCXX__) // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61458
template <size_t Cap>
union aligned_storage_helper {
    struct double1 {
        double a;
    };
    struct double4 {
        double a[4];
    };
    template <class T>
    using maybe = libcxx::conditional_t<(Cap >= sizeof(T)), T, char>;
    char real_data[Cap];
    maybe<int> a;
    maybe<long> b;
    maybe<long long> c;
    maybe<void *> d;
    maybe<void (*)()> e;
    maybe<double1> f;
    maybe<double4> g;
    maybe<long double> h;
};

template <size_t Cap,
          size_t Align =
              libcxx::alignment_of<aligned_storage_helper<Cap>>::value>
struct aligned_storage {
    using type = libcxx::aligned_storage_t<Cap, Align>;
};

template <size_t Cap,
          size_t Align =
              libcxx::alignment_of<aligned_storage_helper<Cap>>::value>
using aligned_storage_t = typename aligned_storage<Cap, Align>::type;
#else
using libcxx::aligned_storage;
using libcxx::aligned_storage_t;
#endif

template <typename T>
struct wrapper {
    using type = T;
};

template <typename R, typename... Args>
struct vtable {
    using storage_ptr_t = void *;

    using invoke_ptr_t     = R (*)(storage_ptr_t, Args &&...);
    using process_ptr_t    = void (*)(storage_ptr_t, storage_ptr_t);
    using destructor_ptr_t = void (*)(storage_ptr_t);

    const invoke_ptr_t invoke_ptr;
    const process_ptr_t copy_ptr;
    const process_ptr_t relocate_ptr;
    const destructor_ptr_t destructor_ptr;

    explicit constexpr vtable() noexcept
        : invoke_ptr{[](storage_ptr_t, Args &&...) -> R {
            // TODO: Probably should abort here
            // throw libcxx::bad_function_call();
        }}
        , copy_ptr{[](storage_ptr_t, storage_ptr_t) noexcept->void{}}
        , relocate_ptr{[](storage_ptr_t, storage_ptr_t) noexcept->void{}}
        , destructor_ptr{[](storage_ptr_t) noexcept->void{}}
    {
    }

    template <typename C>
    explicit constexpr vtable(wrapper<C>) noexcept
        : invoke_ptr{[](storage_ptr_t storage_ptr, Args && ... args) noexcept(
                         noexcept(libcxx::declval<C>()(args...)))
                         ->R{return (*static_cast<C *>(storage_ptr))(
                             libcxx::forward<Args>(args)...);
}
} // namespace function_detail
,
    copy_ptr{
        [](storage_ptr_t dst_ptr, storage_ptr_t src_ptr) -> void {
            ::new (dst_ptr) C{(*static_cast<C *>(src_ptr))};
        } // namespace libcxx
    },
    relocate_ptr{
        [](storage_ptr_t dst_ptr, storage_ptr_t src_ptr) -> void {
            ::new (dst_ptr) C{libcxx::move(*static_cast<C *>(src_ptr))};
            static_cast<C *>(src_ptr)->~C();
        } // namespace libcxx
    },
    destructor_ptr{[](storage_ptr_t src_ptr) noexcept->void{
        static_cast<C *>(src_ptr)->~C();
} // namespace libcxx
}
{
}

vtable(const vtable &) = delete;
vtable(vtable &&)      = delete;

vtable &operator=(const vtable &) = delete;
vtable &operator=(vtable &&) = delete;

~vtable() = default;
}
;

template <typename R, typename... Args>
#if __cplusplus >= 201703L
inline constexpr
#endif
    vtable<R, Args...>
        empty_vtable{};

template <size_t DstCap, size_t DstAlign, size_t SrcCap, size_t SrcAlign>
struct is_valid_inplace_dst : libcxx::true_type {
    static_assert(DstCap >= SrcCap,
                  "Can't squeeze larger function into a smaller one");

    static_assert(DstAlign % SrcAlign == 0, "Incompatible function alignments");
};

} // namespace function_detail

template <typename Signature,
          size_t Capacity  = function_detail::InplaceFunctionDefaultCapacity,
          size_t Alignment = libcxx::alignment_of<
              function_detail::aligned_storage_t<Capacity>>::value>
class function; // unspecified

template <typename R, typename... Args, size_t Capacity, size_t Alignment>
class function<R(Args...), Capacity, Alignment>
{
public:
    using capacity  = libcxx::integral_constant<size_t, Capacity>;
    using alignment = libcxx::integral_constant<size_t, Alignment>;

    using storage_t = function_detail::aligned_storage_t<Capacity, Alignment>;
    using vtable_t  = function_detail::vtable<R, Args...>;
    using vtable_ptr_t = const vtable_t *;

    template <typename, size_t, size_t>
    friend class function;

    function() noexcept
        : vtable_ptr_{
              libcxx::addressof(function_detail::empty_vtable<R, Args...>)}
    {
    }

    template <typename T, typename C = libcxx::decay_t<T>,
              typename = libcxx::enable_if_t<
                  !(libcxx::is_same<C, function>::value ||
                    libcxx::is_convertible<C, function>::value)>>
    function(T &&closure)
    {

        static_assert(sizeof(C) <= Capacity,
                      "function cannot be constructed from object with "
                      "this (large) size");

        static_assert(Alignment % libcxx::alignment_of<C>::value == 0,
                      "function cannot be constructed from object with "
                      "this (large) alignment");

        static const vtable_t vt{function_detail::wrapper<C>{}};
        vtable_ptr_ = libcxx::addressof(vt);

        ::new (libcxx::addressof(storage_)) C{libcxx::forward<T>(closure)};
    }

    function(std::nullptr_t) noexcept
        : vtable_ptr_{
              libcxx::addressof(function_detail::empty_vtable<R, Args...>)}
    {
    }

    function(const function &other)
        : vtable_ptr_{other.vtable_ptr_}
    {
        vtable_ptr_->copy_ptr(libcxx::addressof(storage_),
                              libcxx::addressof(other.storage_));
    }

    function(function &&other)
        : vtable_ptr_{libcxx::exchange(
              other.vtable_ptr_,
              libcxx::addressof(function_detail::empty_vtable<R, Args...>))}
    {
        vtable_ptr_->relocate_ptr(libcxx::addressof(storage_),
                                  libcxx::addressof(other.storage_));
    }

    function &operator=(std::nullptr_t) noexcept
    {
        vtable_ptr_->destructor_ptr(libcxx::addressof(storage_));
        vtable_ptr_ =
            libcxx::addressof(function_detail::empty_vtable<R, Args...>);
        return *this;
    }

    function &operator=(const function &other)
    {
        if (this != libcxx::addressof(other)) {
            vtable_ptr_->destructor_ptr(libcxx::addressof(storage_));

            vtable_ptr_ = other.vtable_ptr_;
            vtable_ptr_->copy_ptr(libcxx::addressof(storage_),
                                  libcxx::addressof(other.storage_));
        }
        return *this;
    }

    function &operator=(function &&other)
    {
        if (this != libcxx::addressof(other)) {
            vtable_ptr_->destructor_ptr(libcxx::addressof(storage_));

            vtable_ptr_ = libcxx::exchange(
                other.vtable_ptr_,
                libcxx::addressof(function_detail::empty_vtable<R, Args...>));
            vtable_ptr_->relocate_ptr(libcxx::addressof(storage_),
                                      libcxx::addressof(other.storage_));
        }
        return *this;
    }

    ~function()
    {
        vtable_ptr_->destructor_ptr(libcxx::addressof(storage_));
    }

    R operator()(Args... args) const
    {
        return vtable_ptr_->invoke_ptr(libcxx::addressof(storage_),
                                       libcxx::forward<Args>(args)...);
    }

    constexpr bool operator==(std::nullptr_t) const noexcept
    {
        return !operator bool();
    }

    constexpr bool operator!=(std::nullptr_t) const noexcept
    {
        return operator bool();
    }

    explicit constexpr operator bool() const noexcept
    {
        return vtable_ptr_ !=
               libcxx::addressof(function_detail::empty_vtable<R, Args...>);
    }

    template <size_t Cap, size_t Align>
    operator function<R(Args...), Cap, Align>() const &
    {
        static_assert(
            function_detail::is_valid_inplace_dst<Cap, Align, Capacity,
                                                  Alignment>::value,
            "conversion not allowed");

        return {vtable_ptr_, vtable_ptr_->copy_ptr,
                libcxx::addressof(storage_)};
    }

    template <size_t Cap, size_t Align>
    operator function<R(Args...), Cap, Align>() &&
    {
        static_assert(
            function_detail::is_valid_inplace_dst<Cap, Align, Capacity,
                                                  Alignment>::value,
            "conversion not allowed");

        auto vtable_ptr = libcxx::exchange(
            vtable_ptr_,
            libcxx::addressof(function_detail::empty_vtable<R, Args...>));

        return {vtable_ptr, vtable_ptr->relocate_ptr,
                libcxx::addressof(storage_)};
    }

    void swap(function &other)
    {
        if (this == libcxx::addressof(other))
            return;

        storage_t tmp;
        vtable_ptr_->relocate_ptr(libcxx::addressof(tmp),
                                  libcxx::addressof(storage_));

        other.vtable_ptr_->relocate_ptr(libcxx::addressof(storage_),
                                        libcxx::addressof(other.storage_));

        vtable_ptr_->relocate_ptr(libcxx::addressof(other.storage_),
                                  libcxx::addressof(tmp));

        libcxx::swap(vtable_ptr_, other.vtable_ptr_);
    }

    friend void swap(function &lhs, function &rhs)
    {
        lhs.swap(rhs);
    }

private:
    vtable_ptr_t vtable_ptr_;
    mutable storage_t storage_;

    function(vtable_ptr_t vtable_ptr,
             typename vtable_t::process_ptr_t process_ptr,
             typename vtable_t::storage_ptr_t storage_ptr)
        : vtable_ptr_{vtable_ptr}
    {
        process_ptr(libcxx::addressof(storage_), storage_ptr);
    }
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
    constexpr binder(Fn &&f, Args &&... args) noexcept
        : f_{libcxx::forward<Fn>(f)}
        , argumentList_{libcxx::forward<Args>(args)...}
    {
    }

    template <class... CallArgs>
    constexpr decltype(auto) operator()(CallArgs &&... args)
    {
        return call(libcxx::make_index_sequence<sizeof...(Args)>{},
                    libcxx::forward<CallArgs>(args)...);
    }

private:
    template <class... CallArgs, size_t... Seq>
    constexpr decltype(auto) call(libcxx::index_sequence<Seq...>,
                                  CallArgs &&... args)
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
