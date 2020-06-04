#pragma once

#include <atomic>

class spinlock
{
public:
    spinlock();

    void lock();
    int lock_irq();

    void unlock();
    void unlock_irq(int save);

    spinlock(const spinlock&) = delete;
    spinlock& operator=(const spinlock&) = delete;

private:
    void _lock();
    void _unlock();
    std::atomic_flag locked;
};
