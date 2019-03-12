#pragma once

#include <proc/process.h>
#include <proc/thread.h>
#include <types.h>

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
     * Then, you will manually need to schedule away (by calling yield()) and
     * checking for the wake reason
     */
    int wait(int flags);
    bool insert(int flags);

    bool remove();

    void wakeup();

private:
    libcxx::list<wait_queue_node, &wait_queue_node::node> waiters;
};

void idle();

bool insert(thread* thread);
bool remove(thread* thread);

void init();
void switch_next(struct interrupt_context* ctx);
void yield();

process* get_current_process();
thread* get_current_thread();

pid_t get_free_pid();

bool online();

bool add_process(process* process);
process* find_process(pid_t pid);
bool remove_process(pid_t pid);
}; // namespace scheduler
