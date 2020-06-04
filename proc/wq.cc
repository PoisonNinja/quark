#include <proc/sched.h>
#include <proc/wq.h>

namespace scheduler
{
bool wait_queue::wait(int flags, libcxx::function<bool()> condition)
{
    struct wait_queue_node node;
    bool sig = false;
    while (1) {
        sig = this->prepare(flags, node);
        if (condition() == true) {
            break;
        }
        if ((flags & wait_interruptible) && sig) {
            break;
        }
        scheduler::switch_next();
        this->remove(node);
    }
    this->remove(node);
    return sig;
}

bool wait_queue::prepare(int flags, wait_queue_node& node)
{
    thread* t = scheduler::get_current_thread();
    if (t->is_signal_pending()) {
        return true;
    }
    this->safe_remove(node);
    this->insert(node);
    thread_state state = (flags & wait_interruptible)
                             ? thread_state::SLEEPING_INTERRUPTIBLE
                             : thread_state::SLEEPING_UNINTERRUPTIBLE;
    t->set_state(state);
    return false;
}

void wait_queue::insert(wait_queue_node& node)
{
    thread* t   = scheduler::get_current_thread();
    node.waiter = t;
    this->waiters.push_back(node);
}

void wait_queue::remove(wait_queue_node& node)
{
    node.waiter->set_state(thread_state::RUNNING);
    this->safe_remove(node);
}

void wait_queue::wakeup()
{
    for (auto it = this->waiters.begin(); it != this->waiters.end();) {
        (*it).waiter->set_state(thread_state::RUNNING);
        scheduler::insert((*it).waiter);
        // Erase the element
        it = this->waiters.erase(it);
    }
}

void wait_queue::safe_remove(wait_queue_node& node)
{
    if (node.node.connected()) {
        waiters.erase(waiters.iterator_to(node));
    }
}
} // namespace scheduler
