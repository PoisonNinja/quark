/*
 * Unless otherwise explicitly stated, the code was taken from cppreference.com
 * which licensed the code as of 11/18/2018 under the CC license.
 */

#pragma once

#include <cstddef>

namespace libcxx
{
template <class T>
struct remove_reference {
    using type = T;
};
template <class T>
struct remove_reference<T &> {
    using type = T;
};
template <class T>
struct remove_reference<T &&> {
    using type = T;
};

template <class T>
using remove_reference_t = typename remove_reference<T>::type;

template <class...>
using void_t = void;

template <bool B, class T, class F>
struct conditional {
    typedef T type;
};

template <class T, class F>
struct conditional<false, T, F> {
    typedef F type;
};

template <bool B, class T, class F>
using conditional_t = typename conditional<B, T, F>::type;

template <bool B, class T = void>
struct enable_if {
};

template <class T>
struct enable_if<true, T> {
    typedef T type;
};

template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <class T, T v>
struct integral_constant {
    static constexpr T value = v;
    typedef T value_type;
    typedef integral_constant type; // using injected-class-name
    constexpr operator value_type() const noexcept
    {
        return value;
    }
    constexpr value_type operator()() const noexcept
    {
        return value;
    } // since c++14
};

using true_type  = libcxx::integral_constant<bool, true>;
using false_type = libcxx::integral_constant<bool, false>;

/*
 * From
 * https://stackoverflow.com/questions/17424477/implementation-c14-make-integer-sequence
 * by joki
 */
template <typename Int, Int... Ints>
struct integer_sequence {
    using value_type = Int;
    static constexpr size_t size() noexcept
    {
        return sizeof...(Ints);
    }
};
template <size_t... Indices>
using index_sequence = integer_sequence<size_t, Indices...>;

namespace
{
// Merge two integer sequences, adding an offset to the right-hand side.
template <typename Offset, typename Lhs, typename Rhs>
struct merge;

template <typename Int, Int Offset, Int... Lhs, Int... Rhs>
struct merge<libcxx::integral_constant<Int, Offset>,
             integer_sequence<Int, Lhs...>, integer_sequence<Int, Rhs...>> {
    using type = integer_sequence<Int, Lhs..., (Offset + Rhs)...>;
};

template <typename Int, typename N>
struct log_make_sequence {
    using L    = libcxx::integral_constant<Int, N::value / 2>;
    using R    = libcxx::integral_constant<Int, N::value - L::value>;
    using type = typename merge<L, typename log_make_sequence<Int, L>::type,
                                typename log_make_sequence<Int, R>::type>::type;
};

// An empty sequence.
template <typename Int>
struct log_make_sequence<Int, libcxx::integral_constant<Int, 0>> {
    using type = integer_sequence<Int>;
};

// A single-element sequence.
template <typename Int>
struct log_make_sequence<Int, libcxx::integral_constant<Int, 1>> {
    using type = integer_sequence<Int, 0>;
};
} // namespace

template <typename Int, Int N>
using make_integer_sequence =
    typename log_make_sequence<Int, libcxx::integral_constant<Int, N>>::type;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

template <class T>
struct remove_const {
    typedef T type;
};
template <class T>
struct remove_const<const T> {
    typedef T type;
};

template <class T>
using remove_const_t = typename remove_const<T>::type;

template <class T>
struct remove_volatile {
    typedef T type;
};
template <class T>
struct remove_volatile<volatile T> {
    typedef T type;
};

template <class T>
using remove_volatile_t = typename remove_volatile<T>::type;

template <class T>
struct remove_cv {
    typedef typename libcxx::remove_volatile<
        typename libcxx::remove_const<T>::type>::type type;
};

template <class T>
using remove_cv_t = typename remove_cv<T>::type;

/*
 * From libcxx - Licensed under MIT license
 */
template <class _Tp>
struct __libcpp_union : public false_type {
};
template <class _Tp>
struct is_union : public __libcpp_union<typename remove_cv<_Tp>::type> {
};

template <class T>
struct is_array : libcxx::false_type {
};

template <class T>
struct is_array<T[]> : libcxx::true_type {
};

template <class T, size_t N>
struct is_array<T[N]> : libcxx::true_type {
};

template <class T>
struct remove_extent {
    typedef T type;
};

template <class T>
struct remove_extent<T[]> {
    typedef T type;
};

template <class T, size_t N>
struct remove_extent<T[N]> {
    typedef T type;
};

template <class T>
struct remove_pointer {
    typedef T type;
};
template <class T>
struct remove_pointer<T *> {
    typedef T type;
};
template <class T>
struct remove_pointer<T *const> {
    typedef T type;
};
template <class T>
struct remove_pointer<T *volatile> {
    typedef T type;
};
template <class T>
struct remove_pointer<T *const volatile> {
    typedef T type;
};

template <class T>
using remove_pointer_t = typename remove_pointer<T>::type;

// primary template
template <class>
struct is_function : libcxx::false_type {
};

// specialization for regular functions
template <class Ret, class... Args>
struct is_function<Ret(Args...)> : libcxx::true_type {
};

// specialization for variadic functions such as libcxx::printf
template <class Ret, class... Args>
struct is_function<Ret(Args......)> : libcxx::true_type {
};

// specialization for function types that have cv-qualifiers
template <class Ret, class... Args>
struct is_function<Ret(Args...) const> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) volatile> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const volatile> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) volatile> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const volatile> : libcxx::true_type {
};

// specialization for function types that have ref-qualifiers
template <class Ret, class... Args>
struct is_function<Ret(Args...) &> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const &> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) volatile &> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const volatile &> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) &> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const &> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) volatile &> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const volatile &> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) &&> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const &&> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) volatile &&> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const volatile &&> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) &&> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const &&> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) volatile &&> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const volatile &&> : libcxx::true_type {
};

// specializations for noexcept versions of all the above (C++17 and later)

template <class Ret, class... Args>
struct is_function<Ret(Args...) noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) volatile noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const volatile noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) volatile noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const volatile noexcept>
    : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) & noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const &noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) volatile &noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const volatile &noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) & noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const &noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) volatile &noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const volatile &noexcept>
    : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) && noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const &&noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) volatile &&noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args...) const volatile &&noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) && noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const &&noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) volatile &&noexcept> : libcxx::true_type {
};
template <class Ret, class... Args>
struct is_function<Ret(Args......) const volatile &&noexcept>
    : libcxx::true_type {
};

namespace detail
{
template <class T, bool is_function_type = false>
struct add_pointer {
    using type = typename libcxx::remove_reference<T>::type *;
};

template <class T>
struct add_pointer<T, true> {
    using type = T;
};

template <class T, class... Args>
struct add_pointer<T(Args...), true> {
    using type = T (*)(Args...);
};

template <class T, class... Args>
struct add_pointer<T(Args..., ...), true> {
    using type = T (*)(Args..., ...);
};

} // namespace detail

template <class T>
struct add_pointer : detail::add_pointer<T, libcxx::is_function<T>::value> {
};

template <class T, class U>
struct is_same : libcxx::false_type {
};

template <class T>
struct is_same<T, T> : libcxx::true_type {
};

template <class T>
struct is_void : libcxx::is_same<void, typename libcxx::remove_cv<T>::type> {
};

template <class T>
struct is_reference : libcxx::false_type {
};
template <class T>
struct is_reference<T &> : libcxx::true_type {
};
template <class T>
struct is_reference<T &&> : libcxx::true_type {
};

namespace detail
{
template <class T>
char test(int T::*);
struct two {
    char c[2];
};
template <class T>
two test(...);
} // namespace detail

template <class T>
struct is_class
    : libcxx::integral_constant<bool, sizeof(detail::test<T>(0)) == 1 &&
                                          !libcxx::is_union<T>::value> {
};

template <class T>
struct is_member_pointer_helper : libcxx::false_type {
};

template <class T, class U>
struct is_member_pointer_helper<T U::*> : libcxx::true_type {
};

template <class T>
struct is_member_pointer
    : is_member_pointer_helper<typename libcxx::remove_cv<T>::type> {
};

template <class T>
inline constexpr bool is_member_pointer_v = is_member_pointer<T>::value;

template <class T>
struct is_member_function_pointer_helper : libcxx::false_type {
};

template <class T, class U>
struct is_member_function_pointer_helper<T U::*> : libcxx::is_function<T> {
};

template <class T>
struct is_member_function_pointer
    : is_member_function_pointer_helper<libcxx::remove_cv_t<T>> {
};

template <class T>
inline constexpr bool is_member_function_pointer_v =
    is_member_function_pointer<T>::value;

template <class T>
struct is_member_object_pointer
    : libcxx::integral_constant<
          bool, libcxx::is_member_pointer<T>::value &&
                    !libcxx::is_member_function_pointer<T>::value> {
};

template <class T>
inline constexpr bool is_member_object_pointer_v =
    is_member_object_pointer<T>::value;

/*
 * From Boost - Licensed under Boost Software License
 */
namespace detail
{
template <typename T, bool b>
struct add_rvalue_reference_helper {
    typedef T type;
};

template <typename T>
struct add_rvalue_reference_helper<T, true> {
    typedef T &&type;
};

template <typename T>
struct add_rvalue_reference_imp {
    typedef typename libcxx::detail::add_rvalue_reference_helper<
        T, (is_void<T>::value == false &&
            is_reference<T>::value == false)>::type type;
};
} // namespace detail

template <class T>
struct add_rvalue_reference {
    typedef typename libcxx::detail::add_rvalue_reference_imp<T>::type type;
};

template <class T>
typename libcxx::add_rvalue_reference<T>::type declval();

template <class T>
struct decay {
private:
    typedef typename libcxx::remove_reference<T>::type U;

public:
    typedef typename libcxx::conditional<
        libcxx::is_array<U>::value, typename libcxx::remove_extent<U>::type *,
        typename libcxx::conditional<
            libcxx::is_function<U>::value,
            typename libcxx::add_pointer<U>::type,
            typename libcxx::remove_cv<U>::type>::type>::type type;
};

template <class T>
using decay_t = typename decay<T>::type;

namespace details
{
template <typename Base>
libcxx::true_type is_base_of_test_func(const volatile Base *);
template <typename Base>
libcxx::false_type is_base_of_test_func(const volatile void *);
template <typename Base, typename Derived>
using pre_is_base_of =
    decltype(is_base_of_test_func<Base>(libcxx::declval<Derived *>()));

// with <experimental/type_traits>:
// template <typename Base, typename Derived>
// using pre_is_base_of2 =
// libcxx::experimental::detected_or_t<libcxx::true_type, pre_is_base_of, Base,
// Derived>;
template <typename Base, typename Derived, typename = void>
struct pre_is_base_of2 : public libcxx::true_type {
};
// note libcxx::void_t is a C++17 feature
template <typename Base, typename Derived>
struct pre_is_base_of2<Base, Derived,
                       libcxx::void_t<pre_is_base_of<Base, Derived>>>
    : public pre_is_base_of<Base, Derived> {
};
} // namespace details

template <typename Base, typename Derived>
struct is_base_of
    : public libcxx::conditional_t<
          libcxx::is_class<Base>::value && libcxx::is_class<Derived>::value,
          details::pre_is_base_of2<Base, Derived>, libcxx::false_type> {
};

template <class Base, class Derived>
inline constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;
} // namespace libcxx
