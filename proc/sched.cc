#include <config.h>
#include <cpu/cpu.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/queue.h>
#include <lib/string.h>
#include <mm/virtual.h>
#include <proc/ptable.h>
#include <proc/sched.h>
#include <proc/work.h>

namespace scheduler
{
namespace
{
libcxx::queue<thread, &thread::scheduler_node> run_queue;
thread* current_thread  = nullptr;
process* kernel_process = nullptr;
thread* kidle           = nullptr;

::ptable ptable;

bool _online = false;
} // namespace

void idle(void*)
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
    int interrupt_status = interrupt::save();
    interrupt::disable();

    if (!thread->is_on_rq()) {
        run_queue.push(*thread);
#ifdef CONFIG_SCHED_PARANOID
        for (auto& t : run_queue.get_container()) {
            assert(t.get_tid() != thread->get_tid());
        }
#endif
        thread->put_on_rq();
    }

    interrupt::restore(interrupt_status);
    return true;
}

bool remove(thread* thread)
{
    int interrupt_status = interrupt::save();
    interrupt::disable();

    run_queue.erase(*thread);
    thread->remove_from_rq();

    interrupt::restore(interrupt_status);
    return true;
}

thread* next()
{
    thread* next = nullptr;
    if (run_queue.empty()) {
        next = kidle;
    } else {
        next = &(run_queue.front());
        run_queue.pop();
    }
    return next;
}

void switch_next()
{
    int interrupt_status = interrupt::save();
    interrupt::disable();

    thread* old = current_thread;
    if (old && old != kidle) {
        if (old->get_state() == thread_state::RUNNING) {
            run_queue.push(*old);
        } else {
            old->remove_from_rq();
        }
    }
    thread* next_thread = next();
    if (next_thread == old) {
        interrupt::restore(interrupt_status);
        return;
    }
    if (current_thread) {
        if (current_thread->get_process()->get_address_space() !=
            next_thread->get_process()->get_address_space()) {
            memory::virt::set_address_space_root(
                next_thread->get_process()->get_address_space());
        }
    } else {
        memory::virt::set_address_space_root(
            next_thread->get_process()->get_address_space());
    }
    current_thread = next_thread;
    old->switch_thread(next_thread);
    interrupt::restore(interrupt_status);
}

void sleep(thread_state state)
{
    thread* curr = scheduler::get_current_thread();
    if (state != thread_state::SLEEPING_INTERRUPTIBLE &&
        state != thread_state::SLEEPING_UNINTERRUPTIBLE) {
        log::printk(log::log_level::WARNING,
                    "TID %d attempted to sleep with wrong state\n",
                    curr->get_tid());
    }
    curr->set_state(state);
    switch_next();
}

void init()
{
    // Print a message in case something goes wrong when initializing
    log::printk(log::log_level::INFO, "Initializing scheduler...\n");
    kernel_process = new process(nullptr);
    kernel_process->set_address_space(memory::virt::get_address_space_root());
    // TODO: Move this to architecture specific
    thread* kinit = kernel_process->create_thread();
    kinit->set_state(thread_state::RUNNING);
    /*
     * Set kinit as current_thread, so on the first task switch caused by the
     * timer, it will save the kernel context into the kinit task, but since
     * it's also the ONLY task right now, we will resume kinit, so in essence
     * the kernel just became a thread.
     */
    current_thread = kinit;
    _online        = true;
    log::printk(log::log_level::INFO, "Scheduler initialized\n");
}

void late_init()
{
    kidle = create_kernel_thread(idle, nullptr);
    scheduler::work::init();
}

process* get_current_process()
{
    return current_thread->get_process();
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

thread* create_kernel_thread(void (*entry_point)(void*), void* data)
{
    return create_thread(kernel_process, entry_point, data);
}
} // namespace scheduler
