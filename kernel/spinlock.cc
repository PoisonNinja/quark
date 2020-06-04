#include <cpu/cpu.h>
#include <cpu/interrupt.h>
#include <kernel/lock.h>

spinlock::spinlock()
    : locked(ATOMIC_FLAG_INIT)
{
}

void spinlock::_lock()
{
    // TODO: Skip this step on UP configurations
    // For now we have no way to tell, so keep this in.
    while (locked.test_and_set(std::memory_order_acquire))
        cpu::halt();
}

void spinlock::lock()
{
    // TODO: We also need to disable kernel preemption properly
    this->_lock();
}

int spinlock::lock_irq()
{
    int ret = interrupt::save();
    interrupt::disable();

    this->_lock();

    return ret;
}

void spinlock::_unlock()
{
    locked.clear(std::memory_order_release);
}

void spinlock::unlock()
{
    // TODO: We also need to reenable kernel preemption properly
    this->_unlock();
}

void spinlock::unlock_irq(int save)
{
    this->_unlock();

    interrupt::restore(save);
}
