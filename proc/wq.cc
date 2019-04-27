#include <proc/sched.h>
#include <proc/wq.h>

namespace scheduler
{
int wait_queue::wait(int flags)
{
    thread* t = scheduler::get_current_thread();
    wait_queue_node node(t);
    this->waiters.push_back(node);
    t->state = (flags & wait_interruptible)
                   ? thread_state::SLEEPING_INTERRUPTIBLE
                   : thread_state::SLEEPING_UNINTERRUPTIBLE;
    scheduler::remove(t);
    scheduler::yield();
    int ret = 0;
    // Check if a signal is pending
    if (!node.normal_wake) {
        // This wasn't woken up by waken(), so we need to remove it manually
        this->waiters.erase(this->waiters.iterator_to(node));
        ret = 1;
    }
    return ret;
}

bool wait_queue::insert(int flags)
{
    thread* t             = scheduler::get_current_thread();
    wait_queue_node* node = new wait_queue_node(t);
    this->waiters.push_back(*node);
    t->state = (flags & wait_interruptible)
                   ? thread_state::SLEEPING_INTERRUPTIBLE
                   : thread_state::SLEEPING_UNINTERRUPTIBLE;
    return true;
}

bool wait_queue::remove()
{
    thread* t = scheduler::get_current_thread();
    for (auto it = this->waiters.begin(); it != this->waiters.end(); it++) {
        if ((*it).waiter == t) {
            // Erase the element
            auto ptr = &(*it);
            it       = this->waiters.erase(it);
            delete ptr;
            return true;
        }
    }
    return false;
}

void wait_queue::wakeup()
{
    for (auto it = this->waiters.begin(); it != this->waiters.end();) {
        (*it).normal_wake = true;
        scheduler::insert((*it).waiter);
        // Erase the element
        it = this->waiters.erase(it);
    }
}
} // namespace scheduler