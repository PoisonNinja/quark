#pragma once

#include <proc/process.h>
#include <proc/thread.h>
#include <types.h>

namespace Scheduler
{
constexpr int wait_interruptible = (1 << 0);

struct WaitQueueNode {
    WaitQueueNode(Thread* t)
        : thread(t)
        , normal_wake(false){};
    Thread* thread;
    libcxx::Node<WaitQueueNode> node;
    bool normal_wake;
};

class WaitQueue
{
public:
    WaitQueue(){};
    ~WaitQueue(){}; // TODO: We should probably do something when deallocating

    int wait(int flags);

    void wakeup();

private:
    libcxx::List<WaitQueueNode, &WaitQueueNode::node> waiters;
};

void idle();

bool insert(Thread* thread);
bool remove(Thread* thread);

void init();
void switch_next(struct InterruptContext* ctx);
void yield();

Process* get_current_process();
Thread* get_current_thread();

pid_t get_free_pid();

bool online();

bool add_process(Process* process);
Process* find_process(pid_t pid);
bool remove_process(pid_t pid);
}; // namespace Scheduler
