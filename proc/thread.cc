#include <kernel.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/thread.h>

thread::thread(process* p)
{
    parent = p;
    parent->add_thread(this);
    this->signal_count    = 0;
    this->signal_required = false;
    signal::sigemptyset(&this->signal_mask);
    signal::sigemptyset(&this->signal_pending);
}

thread::~thread()
{
}

void thread::exit()
{
    // Only the thread can kill itself
    if (this != scheduler::get_current_thread()) {
        log::printk(log::log_level::WARNING,
                    "Only the thread can kill itself\n");
        return;
    }
    scheduler::remove(this);
    this->parent->remove_thread(this);
    scheduler::yield();
}
