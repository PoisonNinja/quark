#pragma once

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
} // namespace libcxx
