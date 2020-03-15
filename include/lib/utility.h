#pragma once

#include <type_traits>

namespace libcxx
{
/*
 * From cppreference.com
 *
 * Licensed under CC-BY-SA 3.0 - See LICENSE for more details
 */
template <class T>
constexpr typename std::remove_reference<T>::type&& move(T&& t)
{
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}

template <class T>
constexpr T&& forward(typename std::remove_reference<T>::type& t) noexcept
{
    return static_cast<T&&>(t);
}

template <class T>
constexpr T&& forward(typename std::remove_reference<T>::type&& t) noexcept
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

/*
 * Mine :)
 *
 * TODO: Probably just specialize tuple - This was implemented before tuple
 */
template <typename M, typename N>
struct pair {
    constexpr pair()
    {
    }

    template <typename U, typename V,
              typename = std::enable_if_t<std::is_convertible_v<U, M> &&
                                          std::is_convertible_v<V, N>>>
    constexpr pair(const U& u, const V& v)
        : first(u)
        , second(v){};

    template <typename U, typename V,
              typename = std::enable_if_t<std::is_convertible_v<U, M> &&
                                          std::is_convertible_v<V, N>>>
    constexpr pair(U&& u, V&& v)
        : first(libcxx::forward<U>(u))
        , second(libcxx::forward<V>(v)){};

    constexpr pair(const pair& p) = default;
    constexpr pair(pair&& p)      = default;
    constexpr pair& operator=(const pair& other) = default;
    constexpr pair& operator=(pair&& other) = default;

    constexpr bool operator==(const pair<M, N>& rhs)
    {
        return ((this->first == rhs.first) && (this->second == rhs.second));
    }

    constexpr bool operator!=(const pair<M, N>& rhs)
    {
        return !(*this == rhs);
    }

    M first;
    N second;
};

template <class _Tp>
class reference_wrapper;

template <class _Tp>
struct ___make_pair_return {
    typedef _Tp type;
};

template <class _Tp>
struct ___make_pair_return<std::reference_wrapper<_Tp>> {
    typedef _Tp& type;
};

template <class _Tp>
struct __make_pair_return {
    typedef
        typename ___make_pair_return<typename std::decay<_Tp>::type>::type type;
};

template <class _T1, class _T2>
inline pair<typename __make_pair_return<_T1>::type,
            typename __make_pair_return<_T2>::type>
make_pair(_T1&& __t1, _T2&& __t2)
{
    return pair<typename __make_pair_return<_T1>::type,
                typename __make_pair_return<_T2>::type>(
        libcxx::forward<_T1>(__t1), libcxx::forward<_T2>(__t2));
}

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
struct merge<std::integral_constant<Int, Offset>, integer_sequence<Int, Lhs...>,
             integer_sequence<Int, Rhs...>> {
    using type = integer_sequence<Int, Lhs..., (Offset + Rhs)...>;
};

template <typename Int, typename N>
struct log_make_sequence {
    using L    = std::integral_constant<Int, N::value / 2>;
    using R    = std::integral_constant<Int, N::value - L::value>;
    using type = typename merge<L, typename log_make_sequence<Int, L>::type,
                                typename log_make_sequence<Int, R>::type>::type;
};

// An empty sequence.
template <typename Int>
struct log_make_sequence<Int, std::integral_constant<Int, 0>> {
    using type = integer_sequence<Int>;
};

// A single-element sequence.
template <typename Int>
struct log_make_sequence<Int, std::integral_constant<Int, 1>> {
    using type = integer_sequence<Int, 0>;
};
} // namespace

template <typename Int, Int N>
using make_integer_sequence =
    typename log_make_sequence<Int, std::integral_constant<Int, N>>::type;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

/*
 * tuple implementation based on EASTL at
 * https://github.com/electronicarts/EASTL/blob/master/include/EASTL/tuple.h
 * which in turn is based on
 * https://bitbucket.org/mitchnull/samples/src/tip/cpp/Tuple/Tuple.hpp?fileviewer=file-view-default
 *
 * Licensed under modified 3-clause BSD license
 */
template <typename... T>
class tuple;

template <typename Tuple>
class tuple_size;

template <size_t I, typename Tuple>
class tuple_element;

template <size_t I, typename Tuple>
using tuple_element_t = typename tuple_element<I, Tuple>::type;

// const typename for tuple_element_t, for when tuple or TupleImpl cannot itself
// be const
template <size_t I, typename Tuple>
using const_tuple_element_t = typename std::conditional<
    std::is_lvalue_reference<tuple_element_t<I, Tuple>>::value,
    std::add_lvalue_reference_t<
        const std::remove_reference_t<tuple_element_t<I, Tuple>>>,
    const tuple_element_t<I, Tuple>>::type;

// get
template <size_t I, typename... Ts_>
tuple_element_t<I, tuple<Ts_...>>& get(tuple<Ts_...>& t);

template <size_t I, typename... Ts_>
const_tuple_element_t<I, tuple<Ts_...>>& get(const tuple<Ts_...>& t);

template <size_t I, typename... Ts_>
tuple_element_t<I, tuple<Ts_...>>&& get(tuple<Ts_...>&& t);

template <typename T, typename... ts_>
T& get(tuple<ts_...>& t);

template <typename T, typename... ts_>
const T& get(const tuple<ts_...>& t);

template <typename T, typename... ts_>
T&& get(tuple<ts_...>&& t);

// TupleTypes helper
template <typename... Ts>
struct TupleTypes {
};

// tuple_size helper
template <typename T>
class tuple_size
{
};
template <typename T>
class tuple_size<const T> : public tuple_size<T>
{
};
template <typename T>
class tuple_size<volatile T> : public tuple_size<T>
{
};
template <typename T>
class tuple_size<const volatile T> : public tuple_size<T>
{
};

template <typename... Ts>
class tuple_size<TupleTypes<Ts...>>
    : public std::integral_constant<size_t, sizeof...(Ts)>
{
};
template <typename... Ts>
class tuple_size<tuple<Ts...>>
    : public std::integral_constant<size_t, sizeof...(Ts)>
{
};

template <class T>
constexpr size_t tuple_size_v = tuple_size<T>::value;

namespace Internal
{
template <typename TupleIndices, typename... Ts>
struct TupleImpl;
} // namespace Internal

template <typename Indices, typename... Ts>
class tuple_size<Internal::TupleImpl<Indices, Ts...>>
    : public std::integral_constant<size_t, sizeof...(Ts)>
{
};

// tuple_element helper to be able to isolate a type given an index
template <size_t I, typename T>
class tuple_element
{
};

template <size_t I>
class tuple_element<I, TupleTypes<>>
{
public:
    static_assert(I != I, "tuple_element index out of range");
};

template <typename H, typename... Ts>
class tuple_element<0, TupleTypes<H, Ts...>>
{
public:
    typedef H type;
};

template <size_t I, typename H, typename... Ts>
class tuple_element<I, TupleTypes<H, Ts...>>
{
public:
    typedef tuple_element_t<I - 1, TupleTypes<Ts...>> type;
};

// specialization for tuple
template <size_t I, typename... Ts>
class tuple_element<I, tuple<Ts...>>
{
public:
    typedef tuple_element_t<I, TupleTypes<Ts...>> type;
};

template <size_t I, typename... Ts>
class tuple_element<I, const tuple<Ts...>>
{
public:
    typedef typename std::add_const<tuple_element_t<I, TupleTypes<Ts...>>>::type
        type;
};

template <size_t I, typename... Ts>
class tuple_element<I, volatile tuple<Ts...>>
{
public:
    typedef
        typename std::add_volatile<tuple_element_t<I, TupleTypes<Ts...>>>::type
            type;
};

template <size_t I, typename... Ts>
class tuple_element<I, const volatile tuple<Ts...>>
{
public:
    typedef
        typename std::add_cv<tuple_element_t<I, TupleTypes<Ts...>>>::type type;
};

// specialization for TupleImpl
template <size_t I, typename Indices, typename... Ts>
class tuple_element<I, Internal::TupleImpl<Indices, Ts...>>
    : public tuple_element<I, tuple<Ts...>>
{
};

template <size_t I, typename Indices, typename... Ts>
class tuple_element<I, const Internal::TupleImpl<Indices, Ts...>>
    : public tuple_element<I, const tuple<Ts...>>
{
};

template <size_t I, typename Indices, typename... Ts>
class tuple_element<I, volatile Internal::TupleImpl<Indices, Ts...>>
    : public tuple_element<I, volatile tuple<Ts...>>
{
};

template <size_t I, typename Indices, typename... Ts>
class tuple_element<I, const volatile Internal::TupleImpl<Indices, Ts...>>
    : public tuple_element<I, const volatile tuple<Ts...>>
{
};

// attempt to isolate index given a type
template <typename T, typename Tuple>
struct tuple_index {
};

template <typename T>
struct tuple_index<T, TupleTypes<>> {
    typedef void DuplicateTypeCheck;
    tuple_index() = delete; // tuple_index should only be used for compile-time
                            // assistance, and never be instantiated
    static const size_t index = 0;
};

template <typename T, typename... TsRest>
struct tuple_index<T, TupleTypes<T, TsRest...>> {
    typedef int DuplicateTypeCheck;
    // after finding type T in the list of types, try to find type T in TsRest.
    // If we stumble back into this version of tuple_index, i.e. type T appears
    // twice in the list of types, then DuplicateTypeCheck will be of type int,
    // and the static_assert will fail. If we don't, then we'll go through the
    // version of tuple_index above, where all of the types have been exhausted,
    // and DuplicateTypeCheck will be void.
    static_assert(std::is_void<typename tuple_index<
                      T, TupleTypes<TsRest...>>::DuplicateTypeCheck>::value,
                  "duplicate type T in tuple_vector::get<T>(); unique types "
                  "must be provided in declaration, or only use get<size_t>()");

    static const size_t index = 0;
};

template <typename T, typename TsHead, typename... TsRest>
struct tuple_index<T, TupleTypes<TsHead, TsRest...>> {
    typedef typename tuple_index<T, TupleTypes<TsRest...>>::DuplicateTypeCheck
        DuplicateTypeCheck;
    static const size_t index =
        tuple_index<T, TupleTypes<TsRest...>>::index + 1;
};

template <typename T, typename Indices, typename... Ts>
struct tuple_index<T, Internal::TupleImpl<Indices, Ts...>>
    : public tuple_index<T, TupleTypes<Ts...>> {
};

namespace Internal
{

// TupleLeaf
template <size_t I, typename ValueType,
          bool IsEmpty = std::is_empty<ValueType>::value>
class TupleLeaf;

template <size_t I, typename ValueType, bool IsEmpty>
inline void swap(TupleLeaf<I, ValueType, IsEmpty>& a,
                 TupleLeaf<I, ValueType, IsEmpty>& b)
{
    libcxx::swap(a.getInternal(), b.getInternal());
}

template <size_t I, typename ValueType, bool IsEmpty>
class TupleLeaf
{
public:
    TupleLeaf()
        : mValue()
    {
    }
    TupleLeaf(const TupleLeaf&) = default;
    TupleLeaf& operator=(const TupleLeaf&) = delete;

    // We shouldn't need this explicit constructor as it should be handled by
    // the template below but OSX clang is_constructible type trait incorrectly
    // gives false for is_constructible<T&&, T&&>::value
    explicit TupleLeaf(ValueType&& v)
        : mValue(move(v))
    {
    }

    template <typename T,
              typename = typename std::enable_if<
                  std::is_constructible<ValueType, T&&>::value>::type>
    explicit TupleLeaf(T&& t)
        : mValue(forward<T>(t))
    {
    }

    template <typename T>
    explicit TupleLeaf(const TupleLeaf<I, T>& t)
        : mValue(t.getInternal())
    {
    }

    template <typename T>
    TupleLeaf& operator=(T&& t)
    {
        mValue = forward<T>(t);
        return *this;
    }

    int swap(TupleLeaf& t)
    {
        libcxx::Internal::swap(*this, t);
        return 0;
    }

    ValueType& getInternal()
    {
        return mValue;
    }
    const ValueType& getInternal() const
    {
        return mValue;
    }

private:
    ValueType mValue;
};

// Specialize for when ValueType is a reference
template <size_t I, typename ValueType, bool IsEmpty>
class TupleLeaf<I, ValueType&, IsEmpty>
{
public:
    TupleLeaf(const TupleLeaf&) = default;
    TupleLeaf& operator=(const TupleLeaf&) = delete;

    template <typename T,
              typename = typename std::enable_if<
                  std::is_constructible<ValueType, T&&>::value>::type>
    explicit TupleLeaf(T&& t)
        : mValue(forward<T>(t))
    {
    }

    explicit TupleLeaf(ValueType& t)
        : mValue(t)
    {
    }

    template <typename T>
    explicit TupleLeaf(const TupleLeaf<I, T>& t)
        : mValue(t.getInternal())
    {
    }

    template <typename T>
    TupleLeaf& operator=(T&& t)
    {
        mValue = forward<T>(t);
        return *this;
    }

    int swap(TupleLeaf& t)
    {
        libcxx::Internal::swap(*this, t);
        return 0;
    }

    ValueType& getInternal()
    {
        return mValue;
    }
    const ValueType& getInternal() const
    {
        return mValue;
    }

private:
    ValueType& mValue;
};

// TupleLeaf partial specialization for when we can use the Empty Base Class
// Optimization
template <size_t I, typename ValueType>
class TupleLeaf<I, ValueType, true> : private ValueType
{
public:
    // true_type / false_type constructors for case where ValueType is default
    // constructible and should be value initialized and case where it is not
    TupleLeaf(const TupleLeaf&) = default;

    template <typename T,
              typename = typename std::enable_if<
                  std::is_constructible<ValueType, T&&>::value>::type>
    explicit TupleLeaf(T&& t)
        : ValueType(forward<T>(t))
    {
    }

    template <typename T>
    explicit TupleLeaf(const TupleLeaf<I, T>& t)
        : ValueType(t.getInternal())
    {
    }

    template <typename T>
    TupleLeaf& operator=(T&& t)
    {
        ValueType::operator=(forward<T>(t));
        return *this;
    }

    int swap(TupleLeaf& t)
    {
        libcxx::Internal::swap(*this, t);
        return 0;
    }

    ValueType& getInternal()
    {
        return static_cast<ValueType&>(*this);
    }
    const ValueType& getInternal() const
    {
        return static_cast<const ValueType&>(*this);
    }

private:
    TupleLeaf& operator=(const TupleLeaf&) = delete;
};

// MakeTupleTypes

template <typename TupleTypes, typename Tuple, size_t Start, size_t End>
struct MakeTupleTypesImpl;

template <typename... Types, typename Tuple, size_t Start, size_t End>
struct MakeTupleTypesImpl<TupleTypes<Types...>, Tuple, Start, End> {
    typedef typename std::remove_reference<Tuple>::type TupleType;
    typedef typename MakeTupleTypesImpl<
        TupleTypes<Types..., typename std::conditional<
                                 std::is_lvalue_reference<Tuple>::value,
                                 // append ref if Tuple is ref
                                 tuple_element_t<Start, TupleType>&,
                                 // append non-ref otherwise
                                 tuple_element_t<Start, TupleType>>::type>,
        Tuple, Start + 1, End>::type type;
};

template <typename... Types, typename Tuple, size_t End>
struct MakeTupleTypesImpl<TupleTypes<Types...>, Tuple, End, End> {
    typedef TupleTypes<Types...> type;
};

template <typename Tuple>
using MakeTupleTypes_t = typename MakeTupleTypesImpl<
    TupleTypes<>, Tuple, 0,
    tuple_size<typename std::remove_reference<Tuple>::type>::value>::type;

// TupleImpl

template <typename... Ts>
void swallow(Ts&&...)
{
}

template <size_t I, typename Indices, typename... Ts>
tuple_element_t<I, TupleImpl<Indices, Ts...>>&
get(TupleImpl<Indices, Ts...>& t);

template <size_t I, typename Indices, typename... Ts>
const_tuple_element_t<I, TupleImpl<Indices, Ts...>>&
get(const TupleImpl<Indices, Ts...>& t);

template <size_t I, typename Indices, typename... Ts>
tuple_element_t<I, TupleImpl<Indices, Ts...>>&&
get(TupleImpl<Indices, Ts...>&& t);

template <typename T, typename Indices, typename... Ts>
T& get(TupleImpl<Indices, Ts...>& t);

template <typename T, typename Indices, typename... Ts>
const T& get(const TupleImpl<Indices, Ts...>& t);

template <typename T, typename Indices, typename... Ts>
T&& get(TupleImpl<Indices, Ts...>&& t);

template <size_t... Indices, typename... Ts>
struct TupleImpl<libcxx::integer_sequence<size_t, Indices...>, Ts...>
    : public TupleLeaf<Indices, Ts>... {
    constexpr TupleImpl() = default;

    // index_sequence changed to integer_sequence due to issues described below
    // in VS2015 CTP 6.
    // https://connect.microsoft.com/VisualStudio/feedback/details/1126958/error-in-template-parameter-pack-expansion-of-std-index-sequence
    //
    template <typename... Us, typename... ValueTypes>
    explicit TupleImpl(libcxx::integer_sequence<size_t, Indices...>,
                       TupleTypes<Us...>, ValueTypes&&... values)
        : TupleLeaf<Indices, Ts>(forward<ValueTypes>(values))...
    {
    }

    template <typename OtherTuple>
    TupleImpl(OtherTuple&& t)
        : TupleLeaf<Indices, Ts>(
              forward<tuple_element_t<Indices, MakeTupleTypes_t<OtherTuple>>>(
                  get<Indices>(t)))...
    {
    }

    template <typename OtherTuple>
    TupleImpl& operator=(OtherTuple&& t)
    {
        swallow(TupleLeaf<Indices, Ts>::operator=(
            forward<tuple_element_t<Indices, MakeTupleTypes_t<OtherTuple>>>(
                get<Indices>(t)))...);
        return *this;
    }

    TupleImpl& operator=(const TupleImpl& t)
    {
        swallow(TupleLeaf<Indices, Ts>::operator=(
            static_cast<const TupleLeaf<Indices, Ts>&>(t).getInternal())...);
        return *this;
    }

    void swap(TupleImpl& t)
    {
        swallow(TupleLeaf<Indices, Ts>::swap(
            static_cast<TupleLeaf<Indices, Ts>&>(t))...);
    }
};

template <size_t I, typename Indices, typename... Ts>
inline tuple_element_t<I, TupleImpl<Indices, Ts...>>&
get(TupleImpl<Indices, Ts...>& t)
{
    typedef tuple_element_t<I, TupleImpl<Indices, Ts...>> Type;
    return static_cast<Internal::TupleLeaf<I, Type>&>(t).getInternal();
}

template <size_t I, typename Indices, typename... Ts>
inline const_tuple_element_t<I, TupleImpl<Indices, Ts...>>&
get(const TupleImpl<Indices, Ts...>& t)
{
    typedef tuple_element_t<I, TupleImpl<Indices, Ts...>> Type;
    return static_cast<const Internal::TupleLeaf<I, Type>&>(t).getInternal();
}

template <size_t I, typename Indices, typename... Ts>
inline tuple_element_t<I, TupleImpl<Indices, Ts...>>&&
get(TupleImpl<Indices, Ts...>&& t)
{
    typedef tuple_element_t<I, TupleImpl<Indices, Ts...>> Type;
    return static_cast<Type&&>(
        static_cast<Internal::TupleLeaf<I, Type>&>(t).getInternal());
}

template <typename T, typename Indices, typename... Ts>
inline T& get(TupleImpl<Indices, Ts...>& t)
{
    typedef tuple_index<T, TupleImpl<Indices, Ts...>> Index;
    return static_cast<Internal::TupleLeaf<Index::index, T>&>(t).getInternal();
}

template <typename T, typename Indices, typename... Ts>
inline const T& get(const TupleImpl<Indices, Ts...>& t)
{
    typedef tuple_index<T, TupleImpl<Indices, Ts...>> Index;
    return static_cast<const Internal::TupleLeaf<Index::index, T>&>(t)
        .getInternal();
}

template <typename T, typename Indices, typename... Ts>
inline T&& get(TupleImpl<Indices, Ts...>&& t)
{
    typedef tuple_index<T, TupleImpl<Indices, Ts...>> Index;
    return static_cast<T&&>(
        static_cast<Internal::TupleLeaf<Index::index, T>&>(t).getInternal());
}

// TupleLike

template <typename T>
struct TupleLike : public std::false_type {
};

template <typename T>
struct TupleLike<const T> : public TupleLike<T> {
};

template <typename T>
struct TupleLike<volatile T> : public TupleLike<T> {
};

template <typename T>
struct TupleLike<const volatile T> : public TupleLike<T> {
};

template <typename... Ts>
struct TupleLike<tuple<Ts...>> : public std::true_type {
};

template <typename First, typename Second>
struct TupleLike<libcxx::pair<First, Second>> : public std::true_type {
};

// TupleConvertible

template <bool IsSameSize, typename From, typename To>
struct TupleConvertibleImpl : public std::false_type {
};

template <typename... FromTypes, typename... ToTypes>
struct TupleConvertibleImpl<true, TupleTypes<FromTypes...>,
                            TupleTypes<ToTypes...>>
    : public std::integral_constant<
          bool,
          std::conjunction<std::is_convertible<FromTypes, ToTypes>...>::value> {
};

template <typename From, typename To,
          bool = TupleLike<typename std::remove_reference<From>::type>::value,
          bool = TupleLike<typename std::remove_reference<To>::type>::value>
struct TupleConvertible : public std::false_type {
};

template <typename From, typename To>
struct TupleConvertible<From, To, true, true>
    : public TupleConvertibleImpl<
          tuple_size<typename std::remove_reference<From>::type>::value ==
              tuple_size<typename std::remove_reference<To>::type>::value,
          MakeTupleTypes_t<From>, MakeTupleTypes_t<To>> {
};

// TupleAssignable

template <bool IsSameSize, typename Target, typename From>
struct TupleAssignableImpl : public std::false_type {
};

template <typename... TargetTypes, typename... FromTypes>
struct TupleAssignableImpl<true, TupleTypes<TargetTypes...>,
                           TupleTypes<FromTypes...>>
    : public std::bool_constant<std::conjunction<
          std::is_assignable<TargetTypes, FromTypes>...>::value> {
};

template <typename Target, typename From,
          bool = TupleLike<typename std::remove_reference<Target>::type>::value,
          bool = TupleLike<typename std::remove_reference<From>::type>::value>
struct TupleAssignable : public std::false_type {
};

template <typename Target, typename From>
struct TupleAssignable<Target, From, true, true>
    : public TupleAssignableImpl<
          tuple_size<typename std::remove_reference<Target>::type>::value ==
              tuple_size<typename std::remove_reference<From>::type>::value,
          MakeTupleTypes_t<Target>, MakeTupleTypes_t<From>> {
};

// TupleImplicitlyConvertible and TupleExplicitlyConvertible - helpers for
// constraining conditionally-explicit ctors

template <bool IsSameSize, typename TargetType, typename... FromTypes>
struct TupleImplicitlyConvertibleImpl : public std::false_type {
};

template <typename... TargetTypes, typename... FromTypes>
struct TupleImplicitlyConvertibleImpl<true, TupleTypes<TargetTypes...>,
                                      FromTypes...>
    : public std::conjunction<std::is_constructible<TargetTypes, FromTypes>...,
                              std::is_convertible<FromTypes, TargetTypes>...> {
};

template <typename TargetTupleType, typename... FromTypes>
struct TupleImplicitlyConvertible
    : public TupleImplicitlyConvertibleImpl<
          tuple_size<TargetTupleType>::value == sizeof...(FromTypes),
          MakeTupleTypes_t<TargetTupleType>, FromTypes...>::type {
};

template <typename TargetTupleType, typename... FromTypes>
using TupleImplicitlyConvertible_t = std::enable_if_t<
    TupleImplicitlyConvertible<TargetTupleType, FromTypes...>::value, bool>;

template <bool IsSameSize, typename TargetType, typename... FromTypes>
struct TupleExplicitlyConvertibleImpl : public std::false_type {
};

template <typename... TargetTypes, typename... FromTypes>
struct TupleExplicitlyConvertibleImpl<true, TupleTypes<TargetTypes...>,
                                      FromTypes...>
    : public std::conjunction<
          std::is_constructible<TargetTypes, FromTypes>...,
          std::negation<std::conjunction<
              std::is_convertible<FromTypes, TargetTypes>...>>> {
};

template <typename TargetTupleType, typename... FromTypes>
struct TupleExplicitlyConvertible
    : public TupleExplicitlyConvertibleImpl<
          tuple_size<TargetTupleType>::value == sizeof...(FromTypes),
          MakeTupleTypes_t<TargetTupleType>, FromTypes...>::type {
};

template <typename TargetTupleType, typename... FromTypes>
using TupleExplicitlyConvertible_t = std::enable_if_t<
    TupleExplicitlyConvertible<TargetTupleType, FromTypes...>::value, bool>;

// TupleEqual

template <size_t I>
struct TupleEqual {
    template <typename Tuple1, typename Tuple2>
    bool operator()(const Tuple1& t1, const Tuple2& t2)
    {
        static_assert(tuple_size<Tuple1>::value == tuple_size<Tuple2>::value,
                      "comparing tuples of different sizes.");
        return TupleEqual<I - 1>()(t1, t2) && get<I - 1>(t1) == get<I - 1>(t2);
    }
};

template <>
struct TupleEqual<0> {
    template <typename Tuple1, typename Tuple2>
    bool operator()(const Tuple1&, const Tuple2&)
    {
        return true;
    }
};

// TupleLess

template <size_t I>
struct TupleLess {
    template <typename Tuple1, typename Tuple2>
    bool operator()(const Tuple1& t1, const Tuple2& t2)
    {
        static_assert(tuple_size<Tuple1>::value == tuple_size<Tuple2>::value,
                      "comparing tuples of different sizes.");
        return TupleLess<I - 1>()(t1, t2) ||
               (!TupleLess<I - 1>()(t2, t1) && get<I - 1>(t1) < get<I - 1>(t2));
    }
};

template <>
struct TupleLess<0> {
    template <typename Tuple1, typename Tuple2>
    bool operator()(const Tuple1&, const Tuple2&)
    {
        return false;
    }
};

// template <typename T>
// struct MakeTupleReturnImpl {
//     typedef T type;
// };

// template <typename T>
// struct MakeTupleReturnImpl<reference_wrapper<T>> {
//     typedef T& type;
// };

// template <typename T>
// using MakeTupleReturn_t =
//     typename MakeTupleReturnImpl<typename decay<T>::type>::type;

struct ignore_t {
    ignore_t()
    {
    }

    template <typename T>
    const ignore_t& operator=(const T&) const
    {
        return *this;
    }
};

// tuple_cat helpers
template <typename Tuple1, typename Is1, typename Tuple2, typename Is2>
struct TupleCat2Impl;

template <typename... T1s, size_t... I1s, typename... T2s, size_t... I2s>
struct TupleCat2Impl<tuple<T1s...>, index_sequence<I1s...>, tuple<T2s...>,
                     index_sequence<I2s...>> {
    typedef tuple<T1s..., T2s...> ResultType;

    template <typename Tuple1, typename Tuple2>
    static inline ResultType DoCat2(Tuple1&& t1, Tuple2&& t2)
    {
        return ResultType(get<I1s>(forward<Tuple1>(t1))...,
                          get<I2s>(forward<Tuple2>(t2))...);
    }
};

template <typename Tuple1, typename Tuple2>
struct TupleCat2;

template <typename... T1s, typename... T2s>
struct TupleCat2<tuple<T1s...>, tuple<T2s...>> {
    typedef make_index_sequence<sizeof...(T1s)> Is1;
    typedef make_index_sequence<sizeof...(T2s)> Is2;
    typedef TupleCat2Impl<tuple<T1s...>, Is1, tuple<T2s...>, Is2> TCI;
    typedef typename TCI::ResultType ResultType;

    template <typename Tuple1, typename Tuple2>
    static inline ResultType DoCat2(Tuple1&& t1, Tuple2&& t2)
    {
        return TCI::DoCat2(forward<Tuple1>(t1), forward<Tuple2>(t2));
    }
};

template <typename... Tuples>
struct TupleCat;

template <typename Tuple1, typename Tuple2, typename... TuplesRest>
struct TupleCat<Tuple1, Tuple2, TuplesRest...> {
    typedef typename TupleCat2<Tuple1, Tuple2>::ResultType FirstResultType;
    typedef typename TupleCat<FirstResultType, TuplesRest...>::ResultType
        ResultType;

    template <typename TupleArg1, typename TupleArg2, typename... TupleArgsRest>
    static inline ResultType DoCat(TupleArg1&& t1, TupleArg2&& t2,
                                   TupleArgsRest&&... ts)
    {
        return TupleCat<FirstResultType, TuplesRest...>::DoCat(
            TupleCat2<TupleArg1, TupleArg2>::DoCat2(forward<TupleArg1>(t1),
                                                    forward<TupleArg2>(t2)),
            forward<TupleArgsRest>(ts)...);
    }
};

template <typename Tuple1, typename Tuple2>
struct TupleCat<Tuple1, Tuple2> {
    typedef typename TupleCat2<Tuple1, Tuple2>::ResultType ResultType;

    template <typename TupleArg1, typename TupleArg2>
    static inline ResultType DoCat(TupleArg1&& t1, TupleArg2&& t2)
    {
        return TupleCat2<TupleArg1, TupleArg2>::DoCat2(forward<TupleArg1>(t1),
                                                       forward<TupleArg2>(t2));
    }
};

} // namespace Internal

template <typename... Ts>
class tuple;

template <typename T, typename... Ts>
class tuple<T, Ts...>
{
public:
    constexpr tuple() = default;

    template <typename T2 = T, Internal::TupleImplicitlyConvertible_t<
                                   tuple, const T2&, const Ts&...> = 0>
    constexpr tuple(const T& t, const Ts&... ts)
        : mImpl(make_index_sequence<sizeof...(Ts) + 1>{},
                Internal::MakeTupleTypes_t<tuple>{}, t, ts...)
    {
    }

    template <typename T2 = T, Internal::TupleExplicitlyConvertible_t<
                                   tuple, const T2&, const Ts&...> = 0>
    explicit constexpr tuple(const T& t, const Ts&... ts)
        : mImpl(make_index_sequence<sizeof...(Ts) + 1>{},
                Internal::MakeTupleTypes_t<tuple>{}, t, ts...)
    {
    }

    template <typename U, typename... Us,
              Internal::TupleImplicitlyConvertible_t<tuple, U, Us...> = 0>
    constexpr tuple(U&& u, Us&&... us)
        : mImpl(make_index_sequence<sizeof...(Us) + 1>{},
                Internal::MakeTupleTypes_t<tuple>{}, forward<U>(u),
                forward<Us>(us)...)
    {
    }

    template <typename U, typename... Us,
              Internal::TupleExplicitlyConvertible_t<tuple, U, Us...> = 0>
    explicit constexpr tuple(U&& u, Us&&... us)
        : mImpl(make_index_sequence<sizeof...(Us) + 1>{},
                Internal::MakeTupleTypes_t<tuple>{}, forward<U>(u),
                forward<Us>(us)...)
    {
    }

    template <typename OtherTuple,
              typename std::enable_if<
                  Internal::TupleConvertible<OtherTuple, tuple>::value,
                  bool>::type = false>
    tuple(OtherTuple&& t)
        : mImpl(forward<OtherTuple>(t))
    {
    }

    template <typename OtherTuple,
              typename std::enable_if<
                  Internal::TupleAssignable<tuple, OtherTuple>::value,
                  bool>::type = false>
    tuple& operator=(OtherTuple&& t)
    {
        mImpl.operator=(forward<OtherTuple>(t));
        return *this;
    }

    void swap(tuple& t)
    {
        mImpl.swap(t.mImpl);
    }

private:
    typedef Internal::TupleImpl<make_index_sequence<sizeof...(Ts) + 1>, T,
                                Ts...>
        Impl;
    Impl mImpl;

    template <size_t I, typename... Ts_>
    friend tuple_element_t<I, tuple<Ts_...>>& get(tuple<Ts_...>& t);

    template <size_t I, typename... Ts_>
    friend const_tuple_element_t<I, tuple<Ts_...>>& get(const tuple<Ts_...>& t);

    template <size_t I, typename... Ts_>
    friend tuple_element_t<I, tuple<Ts_...>>&& get(tuple<Ts_...>&& t);

    template <typename T_, typename... ts_>
    friend T_& get(tuple<ts_...>& t);

    template <typename T_, typename... ts_>
    friend const T_& get(const tuple<ts_...>& t);

    template <typename T_, typename... ts_>
    friend T_&& get(tuple<ts_...>&& t);
};

template <>
class tuple<>
{
public:
    void swap(tuple&)
    {
    }
};

template <size_t I, typename... Ts>
inline tuple_element_t<I, tuple<Ts...>>& get(tuple<Ts...>& t)
{
    return get<I>(t.mImpl);
}

template <size_t I, typename... Ts>
inline const_tuple_element_t<I, tuple<Ts...>>& get(const tuple<Ts...>& t)
{
    return get<I>(t.mImpl);
}

template <size_t I, typename... Ts>
inline tuple_element_t<I, tuple<Ts...>>&& get(tuple<Ts...>&& t)
{
    return get<I>(move(t.mImpl));
}

template <typename T, typename... Ts>
inline T& get(tuple<Ts...>& t)
{
    return get<T>(t.mImpl);
}

template <typename T, typename... Ts>
inline const T& get(const tuple<Ts...>& t)
{
    return get<T>(t.mImpl);
}

template <typename T, typename... Ts>
inline T&& get(tuple<Ts...>&& t)
{
    return get<T>(move(t.mImpl));
}

template <typename... Ts>
inline void swap(tuple<Ts...>& a, tuple<Ts...>& b)
{
    a.swap(b);
}

template <typename... T1s, typename... T2s>
inline bool operator==(const tuple<T1s...>& t1, const tuple<T2s...>& t2)
{
    return Internal::TupleEqual<sizeof...(T1s)>()(t1, t2);
}

template <typename... T1s, typename... T2s>
inline bool operator!=(const tuple<T1s...>& t1, const tuple<T2s...>& t2)
{
    return !(t1 == t2);
}

template <typename... T1s, typename... T2s>
inline bool operator<(const tuple<T1s...>& t1, const tuple<T2s...>& t2)
{
    return Internal::TupleLess<sizeof...(T1s)>()(t1, t2);
}

template <typename... T1s, typename... T2s>
inline bool operator>(const tuple<T1s...>& t1, const tuple<T2s...>& t2)
{
    return t2 < t1;
}

template <typename... T1s, typename... T2s>
inline bool operator<=(const tuple<T1s...>& t1, const tuple<T2s...>& t2)
{
    return !(t2 < t1);
}

template <typename... T1s, typename... T2s>
inline bool operator>=(const tuple<T1s...>& t1, const tuple<T2s...>& t2)
{
    return !(t1 < t2);
}

// // Tuple helper functions

// template <typename... Ts>
// inline constexpr tuple<Internal::MakeTupleReturn_t<Ts>...>
// make_tuple(Ts&&... values)
// {
//     return tuple<Internal::MakeTupleReturn_t<Ts>...>(forward<Ts>(values)...);
// }
template <typename T1, typename T2>
class tuple_size<pair<T1, T2>> : public std::integral_constant<size_t, 2>
{
};

template <typename T1, typename T2>
class tuple_size<const pair<T1, T2>> : public std::integral_constant<size_t, 2>
{
};

template <typename T1, typename T2>
class tuple_element<0, pair<T1, T2>>
{
public:
    typedef T1 type;
};

template <typename T1, typename T2>
class tuple_element<1, pair<T1, T2>>
{
public:
    typedef T2 type;
};

template <typename T1, typename T2>
class tuple_element<0, const pair<T1, T2>>
{
public:
    typedef const T1 type;
};

template <typename T1, typename T2>
class tuple_element<1, const pair<T1, T2>>
{
public:
    typedef const T2 type;
};

template <size_t I>
struct GetPair;

template <>
struct GetPair<0> {
    template <typename T1, typename T2>
    static constexpr T1& getInternal(pair<T1, T2>& p)
    {
        return p.first;
    }

    template <typename T1, typename T2>
    static constexpr const T1& getInternal(const pair<T1, T2>& p)
    {
        return p.first;
    }

    template <typename T1, typename T2>
    static constexpr T1&& getInternal(pair<T1, T2>&& p)
    {
        return forward<T1>(p.first);
    }
};

template <>
struct GetPair<1> {
    template <typename T1, typename T2>
    static constexpr T2& getInternal(pair<T1, T2>& p)
    {
        return p.second;
    }

    template <typename T1, typename T2>
    static constexpr const T2& getInternal(const pair<T1, T2>& p)
    {
        return p.second;
    }

    template <typename T1, typename T2>
    static constexpr T2&& getInternal(pair<T1, T2>&& p)
    {
        return forward<T2>(p.second);
    }
};

template <size_t I, typename T1, typename T2>
tuple_element_t<I, pair<T1, T2>>& get(pair<T1, T2>& p)
{
    return GetPair<I>::getInternal(p);
}

template <size_t I, typename T1, typename T2>
const tuple_element_t<I, pair<T1, T2>>& get(const pair<T1, T2>& p)
{
    return GetPair<I>::getInternal(p);
}

template <size_t I, typename T1, typename T2>
tuple_element_t<I, pair<T1, T2>>&& get(pair<T1, T2>&& p)
{
    return GetPair<I>::getInternal(move(p));
}
/*
 * From cppreference.com
 *
 * Licensed under CC-BY-SA 3.0 - See LICENSE for more details
 */

namespace detail
{
struct ignore_t {
    template <typename T>
    const ignore_t& operator=(const T&) const
    {
        return *this;
    }
};
} // namespace detail
const detail::ignore_t ignore;

template <typename... Args>
auto tie(Args&... args)
{
    return libcxx::tuple<Args&...>(args...);
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
