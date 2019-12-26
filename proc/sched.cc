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
    thread->set_state(thread_state::RUNNABLE);
    run_queue.push_front(*thread);
    return true;
}

bool remove(thread* thread)
{
    run_queue.erase(run_queue.iterator_to(*thread));
    return true;
}

thread* next()
{
    thread* next = nullptr;
    if (run_queue.empty()) {
        next = kidle;
    } else {
        next = &(run_queue.front());
        remove(next);
        run_queue.push_back(*next);
    }
    return next;
}

void switch_next()
{
    thread* old         = current_thread;
    thread* next_thread = next();
    if (next_thread == old) {
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
}

void yield()
{
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
    scheduler::insert(kinit);
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
    kidle = create_kernel_thread(kernel_process, idle, nullptr);
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
} // namespace scheduler
