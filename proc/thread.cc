#include <kernel.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/thread.h>

thread::thread(process* p)
{
    parent = p;
    parent->add_thread(this);
    this->signal_count = 0;
    signal::sigemptyset(&this->signal_mask);
    signal::sigemptyset(&this->signal_pending);
}

thread::~thread()
{
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
