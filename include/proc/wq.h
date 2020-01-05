#pragma once

#include <lib/list.h>

class thread;

namespace scheduler
{
constexpr int wait_interruptible = (1 << 0);

struct wait_queue_node {
    wait_queue_node(thread* t)
        : waiter(t)
        , normal_wake(false){};
    thread* waiter;
    libcxx::node<wait_queue_node> node;
    bool normal_wake;
};

class wait_queue
{
public:
    wait_queue(){};
    ~wait_queue(){}; // TODO: We should probably do something when deallocating

    /*
     * Should you wait or insert?
     *
     * If you only want to wait exclusively on this wait_queue, use wait. It
     * will handle everything for you incl. signals
     *
     * However, if you want to wait on multiple wait_queues (e.g. poll), use
     * insert to register this thread with the wait_queue.
     *
     * Then, you will manually need to schedule away (by calling switch_next())
     * and checking for the wake reason
     */
    int wait(int flags);

    bool insert();
    bool remove();

    void wakeup();

private:
    libcxx::list<wait_queue_node, &wait_queue_node::node> waiters;
};
} // namespace scheduler
