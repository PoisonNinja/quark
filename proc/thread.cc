#include <kernel.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/thread.h>

thread::thread(process* p, tid_t tid)
{
    parent                      = p;
    this->tid                   = tid;
    this->signal_count          = 0;
    this->tcb.kernel_stack_base = this->tcb.kernel_stack =
        reinterpret_cast<addr_t>(new uint8_t[0xF000] + 0xF000) & ~15UL;
    for (int i = 0; i < 1; i++) {
        this->flags[i] = false;
    }
    signal::sigemptyset(&this->signal_mask);
    signal::sigemptyset(&this->signal_pending);
}

thread::~thread()
{
}

tid_t thread::get_tid()
{
    return this->tid;
}

process* thread::get_process()
{
    return this->parent;
}

thread_state thread::get_state()
{
    return this->state;
}

bool thread::get_flag(thread_flag flag)
{
    return this->flags[static_cast<int>(flag)];
}

void thread::set_flag(thread_flag flag, bool value)
{
    this->flags[static_cast<int>(flag)] = value;
}

void thread::set_state(thread_state state)
{
    if (this->state == state) {
        log::printk(
            log::log_level::WARNING,
            "thread: Attempting to set state to same state for thread %u\n",
            this->tid);
    }
    this->state = state;
}

void thread::exit(bool is_signal, int val)
{
    // Only the thread can kill itself
    if (this != scheduler::get_current_thread()) {
        log::printk(log::log_level::WARNING,
                    "Only the thread can kill itself\n");
        return;
    }
    // TODO: We need to clean up!
    scheduler::remove(this);
    this->parent->thread_exit(this, is_signal, val);
    scheduler::yield();
}
