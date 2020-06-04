#pragma once

#include <kernel/mutex.h>
#include <kernel/spinlock.h>
#include <type_traits>

template <class T, class Enable = void>
class scoped_lock;

template <class T>
class scoped_lock<
    T, typename std::enable_if<std::conjunction_v<
           std::is_member_function_pointer<decltype(&T::lock)>,
           std::is_member_function_pointer<decltype(&T::unlock)>>>::type>
{
public:
    scoped_lock(T& mutex)
        : lock(mutex)
    {
        lock.lock();
    };
    ~scoped_lock()
    {
        lock.unlock();
    }

private:
    T& lock;
};

template <class T, class Enable = void>
class scoped_lock_irq;

template <class T>
class scoped_lock_irq<
    T, typename std::enable_if<std::conjunction_v<
           std::is_member_function_pointer<decltype(&T::lock_irq)>,
           std::is_member_function_pointer<decltype(&T::unlock_irq)>>>::type>
{
public:
    scoped_lock_irq(T& mutex)
        : lock(mutex)
    {
        flag = lock.lock_irq();
    };
    ~scoped_lock_irq()
    {
        lock.unlock_irq(flag);
    }

private:
    int flag;
    T& lock;
};
