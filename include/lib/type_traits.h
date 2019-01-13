#pragma once

#include <cstddef>

namespace libcxx
{
/*
 * From cppreference.com
 *
 * Licensed under CC-BY-SA 3.0 - See LICENSE for more details
 */
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

template <bool B>
using bool_constant = integral_constant<bool, B>;

using true_type  = libcxx::integral_constant<bool, true>;
using false_type = libcxx::integral_constant<bool, false>;

/*
 * From
 * https://stackoverflow.com/questions/17424477/implementation-c14-make-integer-sequence
 * by joki
 *
 * Licensed under CC-BY-SA 3.0 - See LICENSE for more details
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

/*
 * From cppreference.com
 *
 * Licensed under CC-BY-SA 3.0 - See LICENSE for more details
 */

template <class T>
struct add_cv {
    typedef const volatile T type;
};

template <class T>
struct add_const {
    typedef const T type;
};

template <class T>
struct add_volatile {
    typedef volatile T type;
};

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
 * From libcxx
 *
 * Licensed under MIT license
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

/*
 * From cppreference.com
 *
 * Licensed under CC-BY-SA 3.0 - See LICENSE for
 */
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

template <class T>
struct is_rvalue_reference : libcxx::false_type {
};

template <class T>
struct is_rvalue_reference<T &&> : libcxx::true_type {
};

template <class T>
struct is_lvalue_reference : libcxx::false_type {
};

template <class T>
struct is_lvalue_reference<T &> : libcxx::true_type {
};

/*
 * From libcxx
 */
struct __two {
    char __lx[2];
};

struct __is_referenceable_impl {
    template <class _Tp>
    static _Tp &__test(int);
    template <class _Tp>
    static __two __test(...);
};

template <class _Tp>
struct __is_referenceable
    : integral_constant<
          bool, !is_same<decltype(__is_referenceable_impl::__test<_Tp>(0)),
                         __two>::value> {
};

template <typename _Tp, bool = __is_referenceable<_Tp>::value>
struct __add_lvalue_reference_helper {
    typedef _Tp type;
};

template <typename _Tp>
struct __add_lvalue_reference_helper<_Tp, true> {
    typedef _Tp &type;
};
/*
 * From Boost
 *
 * Licensed under Boost Software License - See LICENSE for more details
 */
/// add_lvalue_reference
template <typename _Tp>
struct add_lvalue_reference : public __add_lvalue_reference_helper<_Tp> {
};

template <class _Tp>
using add_lvalue_reference_t = typename add_lvalue_reference<_Tp>::type;

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

/*
 * From cppreference.com
 *
 * Licensed under CC-BY-SA 3.0 - See LICENSE for more details
 */
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

template <size_t Len, size_t Align>
struct aligned_storage {
    struct type {
        alignas(Align) unsigned char data[Len];
    };
};

template <size_t Len, size_t Align>
using aligned_storage_t = typename aligned_storage<Len, Align>::type;

template <class T>
struct alignment_of : libcxx::integral_constant<size_t, alignof(T)> {
};

template <class T>
inline constexpr size_t alignment_of_v = alignment_of<T>::value;

/*
 * From libcxx
 *
 * Licensed under MIT license - See LICENSE for more details
 */
namespace __is_convertible_imp
{
template <class _Tp>
void __test_convert(_Tp);

template <class _From, class _To, class = void>
struct __is_convertible_test : public false_type {
};

template <class _From, class _To>
struct __is_convertible_test<
    _From, _To,
    decltype(libcxx::__is_convertible_imp::__test_convert<_To>(
        libcxx::declval<_From>()))> : public true_type {
};

template <class _Tp, bool _IsArray = is_array<_Tp>::value,
          bool _IsFunction = is_function<_Tp>::value,
          bool _IsVoid     = is_void<_Tp>::value>
struct __is_array_function_or_void {
    enum { value = 0 };
};
template <class _Tp>
struct __is_array_function_or_void<_Tp, true, false, false> {
    enum { value = 1 };
};
template <class _Tp>
struct __is_array_function_or_void<_Tp, false, true, false> {
    enum { value = 2 };
};
template <class _Tp>
struct __is_array_function_or_void<_Tp, false, false, true> {
    enum { value = 3 };
};
} // namespace __is_convertible_imp

template <class _Tp,
          unsigned = __is_convertible_imp::__is_array_function_or_void<
              typename remove_reference<_Tp>::type>::value>
struct __is_convertible_check {
    static const size_t __v = 0;
};

template <class _Tp>
struct __is_convertible_check<_Tp, 0> {
    static const size_t __v = sizeof(_Tp);
};

template <class _T1, class _T2,
          unsigned _T1_is_array_function_or_void =
              __is_convertible_imp::__is_array_function_or_void<_T1>::value,
          unsigned _T2_is_array_function_or_void =
              __is_convertible_imp::__is_array_function_or_void<_T2>::value>
struct __is_convertible
    : public integral_constant<
          bool,
          __is_convertible_imp::__is_convertible_test<_T1, _T2>::value
#if defined(_LIBCPP_HAS_NO_RVALUE_REFERENCES)
              &&
              !(!is_function<_T1>::value && !is_reference<_T1>::value &&
                is_reference<_T2>::value &&
                (!is_const<typename remove_reference<_T2>::type>::value ||
                 is_volatile<typename remove_reference<_T2>::type>::value) &&
                (is_same<typename remove_cv<_T1>::type,
                         typename remove_cv<typename remove_reference<
                             _T2>::type>::type>::value ||
                 is_base_of<typename remove_reference<_T2>::type, _T1>::value))
#endif
          >{};

template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 0, 1> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 1, 1> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 2, 1> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 3, 1> : public false_type{};

template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 0, 2> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 1, 2> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 2, 2> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 3, 2> : public false_type{};

template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 0, 3> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 1, 3> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 2, 3> : public false_type{};
template <class _T1, class _T2>
struct __is_convertible<_T1, _T2, 3, 3> : public true_type{};

template <class _T1, class _T2>
struct is_convertible : public __is_convertible<_T1, _T2> {
    static const size_t __complete_check1 = __is_convertible_check<_T1>::__v;
    static const size_t __complete_check2 = __is_convertible_check<_T2>::__v;
};

/*
 * From cppreference.com
 */
template <typename T, typename U, typename = void>
struct is_assignable : libcxx::false_type {
};

template <typename T, typename U>
struct is_assignable<
    T, U, decltype(libcxx::declval<T>() = libcxx::declval<U>(), void())>
    : libcxx::true_type {
};

template <typename _Tp>
struct is_empty : public integral_constant<bool, __is_empty(_Tp)> {
};

template <class _Tp, class... _Args>
struct is_constructible
    : public integral_constant<bool, __is_constructible(_Tp, _Args...)> {
};

template <class...>
struct conjunction : libcxx::true_type {
};
template <class B1>
struct conjunction<B1> : B1 {
};
template <class B1, class... Bn>
struct conjunction<B1, Bn...>
    : libcxx::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {
};

template <class B>
struct negation : libcxx::bool_constant<!bool(B::value)> {
};
} // namespace libcxx
