#include <proc/process.h>
#include <proc/sched.h>
#include <proc/thread.h>

extern void arch_set_stack(addr_t stack);
extern addr_t arch_get_stack();

Thread::Thread(Process* p)
{
    parent = p;
    parent->add_thread(this);
}

Thread::~Thread()
{
}

void __attribute__((noreturn)) Thread::exit()
{
    Scheduler::remove(this);
    this->parent->remove_thread(this);
    Scheduler::yield();
}

void set_stack(addr_t stack)
{
    return arch_set_stack(stack);
}

addr_t get_stack()
{
    return arch_get_stack();
}
