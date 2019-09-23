#include <kernel.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/thread.h>

thread::thread(process* p, tid_t tid)
{
    parent             = p;
    this->tid          = tid;
    this->signal_count = 0;
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
