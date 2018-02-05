#include <proc/process.h>
#include <proc/sched.h>

Process::Process(Process* parent)
{
    this->parent = parent;
    this->pid = Scheduler::get_free_pid();
}

Process::~Process()
{
}

status_t Process::add_thread(Thread* thread)
{
    if (threads.empty()) {
        // First node shares same PID
        thread->tid = this->pid;
    } else {
        // Rest of them increment the PID
        thread->tid = Scheduler::get_free_pid();
    }
    threads.push_back(*thread);
    return SUCCESS;
}

status_t Process::remove_thread(Thread* thread)
{
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        auto& value = *it;
        if (&value == thread) {
            threads.erase(it);
            return SUCCESS;
        }
    }
    return FAILURE;
}
