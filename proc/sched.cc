#include <cpu/cpu.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/list.h>
#include <lib/string.h>
#include <mm/virtual.h>
#include <proc/ptable.h>
#include <proc/sched.h>

namespace scheduler
{
namespace
{
libcxx::list<thread, &thread::scheduler_node> run_queue;
thread* current_thread;
process* kernel_process;
thread* kidle;

ptable ptable;

bool _online = false;
} // namespace

void idle()
{
    while (1) {
        // TODO: Get rid of hlt
        cpu::halt();
    }
}

static pid_t next_pid = 0;

pid_t get_free_pid()
{
    // TODO: Get next __FREE__ slot, not just next slot
    return next_pid++;
}

bool insert(thread* thread)
{
    thread->state = thread_state::RUNNABLE;
    run_queue.push_front(*thread);
    return true;
}

bool remove(thread* thread)
{
    thread->state = thread_state::UNMANAGED;
    run_queue.erase(run_queue.iterator_to(*thread));
    return true;
}

thread* next()
{
    thread* next = nullptr;
    if (run_queue.empty()) {
        kernel::panic("sched: Nothing to run!\n");
    } else {
        next = &(run_queue.front());
        remove(next);
        run_queue.push_back(*next);
    }
    return next;
}

void switch_context(struct interrupt_context* ctx, thread* current,
                    thread* next)
{
    save_context(ctx, &current->tcontext);
    if (current) {
        if (current->parent->address_space != next->parent->address_space) {
            memory::virt::set_address_space_root(next->parent->address_space);
        }
    } else {
        memory::virt::set_address_space_root(next->parent->address_space);
    }
    load_context(ctx, &next->tcontext);
}

void switch_next(struct interrupt_context* ctx)
{
    thread* next_thread = next();
    switch_context(ctx, current_thread, next_thread);
    current_thread = next_thread;
    if (scheduler::online()) {
        if (scheduler::get_current_thread()->signal_required) {
            scheduler::get_current_thread()->handle_signal(ctx);
        }
    }
}

void yield_switch(int, void*, struct interrupt_context* ctx)
{
    switch_next(ctx);
}

interrupt::handler yield_handler(yield_switch, "yield", &yield_handler);

void init()
{
    log::printk(log::log_level::INFO, "Initializing scheduler...\n");
    interrupt::register_handler(0x81, yield_handler);
    kernel_process                = new process(nullptr);
    kernel_process->address_space = memory::virt::get_address_space_root();
    // TODO: Move this to architecture specific
    thread* kinit = new thread(kernel_process);
    scheduler::insert(kinit);
    /*
     * Set kinit as current_thread, so on the first task switch caused by the
     * timer, it will save the kernel context into the kinit task, but since
     * it's also the ONLY task right now, we will resume kinit, so in essence
     * the kernel just became a thread.
     */
    current_thread = kinit;
    _online        = true;
    log::printk(log::log_level::INFO, "scheduler initialized\n");
}

process* get_current_process()
{
    return current_thread->parent;
}

thread* get_current_thread()
{
    return current_thread;
}

bool online()
{
    return _online;
}

bool add_process(process* process)
{
    return ptable.add(process);
}

process* find_process(pid_t pid)
{
    return ptable.get(pid);
}

bool remove_process(pid_t pid)
{
    return ptable.remove(pid);
}
} // namespace scheduler
