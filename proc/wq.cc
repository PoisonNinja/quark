#include <proc/sched.h>
#include <proc/wq.h>

namespace scheduler
{
int wait_queue::wait(int flags)
{
    struct wait_queue_node node;
    this->insert(node);
    thread_state state = (flags & wait_interruptible)
                             ? thread_state::SLEEPING_INTERRUPTIBLE
                             : thread_state::SLEEPING_UNINTERRUPTIBLE;
    scheduler::sleep(state);
    int ret = 0;
    return ret;
}

bool wait_queue::insert(wait_queue_node& node)
{
    thread* t   = scheduler::get_current_thread();
    node.waiter = t;
    this->waiters.push_back(node);
    return true;
}

bool wait_queue::remove(wait_queue_node& node)
{
    // TODO: Validate this somehow
    waiters.erase(waiters.iterator_to(node));
}

void wait_queue::wakeup()
{
    for (auto it = this->waiters.begin(); it != this->waiters.end();) {
        scheduler::insert((*it).waiter);
        auto ptr = &(*it);
        // Erase the element
        it = this->waiters.erase(it);
    }
}
} // namespace scheduler
