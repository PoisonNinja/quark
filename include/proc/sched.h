#pragma once

#include <proc/process.h>
#include <proc/thread.h>
#include <types.h>

namespace Scheduler
{
typedef void* token_t;

class Waiter
{
public:
    Waiter(Thread* t, token_t token, int flags)
        : thread(t), token(token), flags(flags){};
    virtual ~Waiter(){};
    bool operator==(const Waiter& other) const;
    virtual bool check(token_t token);
    virtual Thread* get_thread();
    Node<Waiter> node;

protected:
    Thread* thread;
    token_t token;
    int flags;
};

bool broadcast(token_t token);
void wait(token_t token, int flags);

constexpr int wait_interruptible = (1 << 0);

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
};  // namespace Scheduler
