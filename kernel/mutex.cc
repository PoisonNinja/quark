#include <kernel/lock.h>

mutex::mutex()
    : locked(ATOMIC_FLAG_INIT)
{
}

void mutex::lock()
{
    // For now we have no way to tell, so keep this in.
    while (locked.test_and_set(std::memory_order_acquire)) {
        // We'd rather not be woken up by a signal
        queue.wait(0);
    }
}

void mutex::unlock()
{
    locked.clear(std::memory_order_release);
    queue.wakeup();
}
