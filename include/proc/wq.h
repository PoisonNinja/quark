#pragma once

#include <lib/functional.h>
#include <lib/list.h>

class thread;

namespace scheduler
{
constexpr int wait_interruptible = (1 << 0);

struct wait_queue_node {
    thread* waiter;
    libcxx::node<wait_queue_node> node;
};

class wait_queue
{
public:
    wait_queue(){};
    ~wait_queue(){}; // TODO: We should probably do something when deallocating

    /*
     * Basically handles all aspects of waiting for you. condition must be true
     * when wait is terminating (e.g. waiting for condition to become true).
     *
     * Internally, it'll call prepare and remove on behalf of you.
     */
    bool wait(int flags, libcxx::function<bool()> condition);

    /*
     * If you need more fine grained control, use the below functions.
     */
    /*
     * Inserts the node into the wait queue and set the appropriate thread
     * state. If a signal is pending, it'll abort and return true. Otherwise,
     * it'll insert the node and modify the state so that on the next task
     * switch the thread will not be scheduled.
     */
    bool prepare(int flags, wait_queue_node& node);
    // Only inserts the node into the wait queue
    void insert(wait_queue_node& node);
    /*
     * Removes the node from the wait queue and also sets the current thread
     * state to RUNNABLE.
     */
    void remove(wait_queue_node& node);

    void wakeup();

private:
    /*
     * Safely removes a node from the queue. It'll only remove if the
     * node is actually connected in the list, making it safe to call multiple
     * times with the same node.
     */
    void safe_remove(wait_queue_node& node);

    libcxx::list<wait_queue_node, &wait_queue_node::node> waiters;
};
} // namespace scheduler
