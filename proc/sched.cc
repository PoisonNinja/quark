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
List<Thread, &Thread::scheduler_node> run_queue;
List<Waiter, &Waiter::node> sleep_queue;
Thread* current_thread;
Process* kernel_process;
Thread* kidle;

PTable ptable;

bool _online = false;
}  // namespace

bool Waiter::check(token_t token)
{
    return token == this->token;
}

bool Waiter::operator==(const Waiter& other) const
{
    return this->token == other.token;
}

Thread* Waiter::get_thread()
{
    return this->thread;
}

bool broadcast(token_t token)
{
    for (auto& sleeper : sleep_queue) {
        if (sleeper.check(token)) {
            Log::printk(Log::DEBUG, "Sleeper is ready to be woken, %p\n");
            Scheduler::insert(sleeper.get_thread());
            sleep_queue.remove(sleeper);
            return true;
        }
    }
    return false;
}

void wait(token_t token, int flags)
{
    Waiter* waiter = new Waiter(Scheduler::get_current_thread(), token, flags);
    sleep_queue.push_back(*waiter);
    Scheduler::remove(Scheduler::get_current_thread());
    yield();
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
    thread->state = RUNNABLE;
    run_queue.push_front(*thread);
    return true;
}

bool remove(Thread* thread)
{
    for (auto it = run_queue.begin(); it != run_queue.end(); ++it) {
        auto& value = *it;
        if (&value == thread) {
            thread->state = UNMANAGED;
            run_queue.erase(it);
            return true;
        }
    }
    return false;
}

Thread* next()
{
    Thread* next = nullptr;
    if (run_queue.empty()) {
        next = kidle;
    } else {
        next = &run_queue.front();
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
            Memory::Virtual::set_address_space_root(
                next->parent->address_space);
        }
    } else {
        Memory::Virtual::set_address_space_root(next->parent->address_space);
    }
    load_context(ctx, &next->tcontext);
}

void switch_next(struct InterruptContext* ctx)
{
    Thread* next_thread = next();
    switch_context(ctx, current_thread, next_thread);
    current_thread = next_thread;
}

void yield_switch(int, void*, struct InterruptContext* ctx)
{
    Thread* next_thread = next();
    switch_context(ctx, current_thread, next_thread);
    current_thread = next_thread;
}

Interrupt::Handler yield_handler(yield_switch, "yield", &yield_handler);

void init()
{
    Log::printk(Log::INFO, "Initializing scheduler...\n");
    Interrupt::register_handler(0x81, yield_handler);
    kernel_process = new Process(nullptr);
    kernel_process->address_space = Memory::Virtual::get_address_space_root();
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
    _online = true;
    Log::printk(Log::INFO, "Scheduler initialized\n");
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
}  // namespace Scheduler
