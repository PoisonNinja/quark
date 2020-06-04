#pragma once

#include <atomic>
#include <proc/wq.h>

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
