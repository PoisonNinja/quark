#include <cpu/cpu.h>
#include <kernel/lock.h>

spinlock::spinlock()
    : locked(ATOMIC_FLAG_INIT)
{
}

void spinlock::lock()
{
    // TODO: Skip this step on UP configurations
    // For now we have no way to tell, so keep this in.
    while (locked.test_and_set(std::memory_order_acquire))
        cpu::halt();
    // TODO: We also need to disable kernel preemption properly
}

void spinlock::unlock()
{
    locked.clear(std::memory_order_release);
}
