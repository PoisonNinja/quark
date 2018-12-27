#include <cpu/cpu.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/list.h>
#include <lib/string.h>
#include <mm/virtual.h>
#include <proc/ptable.h>
#include <proc/sched.h>

namespace Scheduler
{
namespace
{
libcxx::list<Thread, &Thread::scheduler_node> run_queue;
Thread* current_thread;
Process* kernel_process;
Thread* kidle;

PTable ptable;

bool _online = false;
} // namespace

int WaitQueue::wait(int flags)
{
    Thread* t = Scheduler::get_current_thread();
    WaitQueueNode node(t);
    this->waiters.push_back(node);
    t->state = (flags & wait_interruptible)
                   ? ThreadState::SLEEPING_INTERRUPTIBLE
                   : ThreadState::SLEEPING_UNINTERRUPTIBLE;
    Scheduler::remove(t);
    Scheduler::yield();
    int ret = 0;
    // Check if a signal is pending
    if (!node.normal_wake) {
        // This wasn't woken up by waken(), so we need to remove it manually
        this->waiters.erase(this->waiters.iterator_to(node));
        ret = 1;
    }
    return ret;
}

void WaitQueue::wakeup()
{
    for (auto it = this->waiters.begin(); it != this->waiters.end();) {
        (*it).normal_wake = true;
        Scheduler::insert((*it).thread);
        // Erase the element
        it = this->waiters.erase(it);
    }
}

void idle()
{
    while (1) {
        // TODO: Get rid of hlt
        CPU::halt();
    }
}

static pid_t next_pid = 0;

pid_t get_free_pid()
{
    // TODO: Get next __FREE__ slot, not just next slot
    return next_pid++;
}

bool insert(Thread* thread)
{
    thread->state = ThreadState::RUNNABLE;
    run_queue.push_front(*thread);
    return true;
}

bool remove(Thread* thread)
{
    thread->state = ThreadState::UNMANAGED;
    run_queue.erase(run_queue.iterator_to(*thread));
    return true;
}

Thread* next()
{
    Thread* next = nullptr;
    if (run_queue.empty()) {
        next = kidle;
    } else {
        next = &(run_queue.front());
        remove(next);
        run_queue.push_back(*next);
    }
    return next;
}

void switch_context(struct InterruptContext* ctx, Thread* current, Thread* next)
{
    save_context(ctx, &current->tcontext);
    if (current) {
        if (current->parent->address_space != next->parent->address_space) {
            memory::Virtual::set_address_space_root(
                next->parent->address_space);
        }
    } else {
        memory::Virtual::set_address_space_root(next->parent->address_space);
    }
    load_context(ctx, &next->tcontext);
}

void switch_next(struct InterruptContext* ctx)
{
    Thread* next_thread = next();
    switch_context(ctx, current_thread, next_thread);
    current_thread = next_thread;
    if (Scheduler::online()) {
        if (Scheduler::get_current_thread()->signal_required) {
            Scheduler::get_current_thread()->handle_signal(ctx);
        }
    }
}

void yield_switch(int, void*, struct InterruptContext* ctx)
{
    switch_next(ctx);
}

Interrupt::Handler yield_handler(yield_switch, "yield", &yield_handler);

void init()
{
    Log::printk(Log::LogLevel::INFO, "Initializing scheduler...\n");
    Interrupt::register_handler(0x81, yield_handler);
    kernel_process                = new Process(nullptr);
    kernel_process->address_space = memory::Virtual::get_address_space_root();
    // TODO: Move this to architecture specific
    Thread* kinit = new Thread(kernel_process);
    Scheduler::insert(kinit);
    /*
     * Set kinit as current_thread, so on the first task switch caused by the
     * timer, it will save the kernel context into the kinit task, but since
     * it's also the ONLY task right now, we will resume kinit, so in essence
     * the kernel just became a thread.
     */
    current_thread = kinit;
    _online        = true;
    Log::printk(Log::LogLevel::INFO, "Scheduler initialized\n");
}

Process* get_current_process()
{
    return current_thread->parent;
}

Thread* get_current_thread()
{
    return current_thread;
}

bool online()
{
    return _online;
}

bool add_process(Process* process)
{
    return ptable.add(process);
}

Process* find_process(pid_t pid)
{
    return ptable.get(pid);
}

bool remove_process(pid_t pid)
{
    return ptable.remove(pid);
}
} // namespace Scheduler
