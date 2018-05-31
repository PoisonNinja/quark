#include <kernel.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/thread.h>

Thread::Thread(Process* p)
{
    parent = p;
    parent->add_thread(this);
    this->signal_count = 0;
    this->signal_required = false;
    Signal::sigemptyset(&this->signal_pending);
}

Thread::~Thread()
{
}

void Thread::exit()
{
    // Only the thread can kill itself
    if (this != Scheduler::get_current_thread()) {
        Log::printk(Log::WARNING, "Only the thread can kill itself\n");
        return;
    }
    Scheduler::remove(this);
    this->parent->remove_thread(this);
    Scheduler::yield();
}
