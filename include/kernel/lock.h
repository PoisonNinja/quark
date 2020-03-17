#pragma once

#include <atomic>
#include <proc/wq.h>
#include <type_traits>

class spinlock
{
public:
    spinlock();
    void lock();
    void unlock();

    spinlock(const spinlock&) = delete;
    spinlock& operator=(const spinlock&) = delete;

private:
    std::atomic_flag locked;
};

class mutex
{
public:
    mutex();
    void lock();
    void unlock();

    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

private:
    std::atomic_flag locked;
    scheduler::wait_queue queue;
};

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
