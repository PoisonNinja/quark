#include <arch/mm/layout.h>
#include <kernel.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/process.h>
#include <proc/sched.h>

process::process(process* parent)
    : pid(scheduler::get_free_pid())
    , vma(new memory::vma(USER_START, USER_END))
    , exit_reason(-1)
    , parent(parent)
{
    for (int i = 1; i < NSIGS; i++) {
        this->signal_actions[i].sa_handler = SIG_DFL;
        this->signal_actions[i].sa_flags   = 0;
    }
}

process::~process()
{
}

void process::set_cwd(libcxx::intrusive_ptr<filesystem::descriptor> desc)
{
    cwd = desc;
}

void process::set_root(libcxx::intrusive_ptr<filesystem::descriptor> desc)
{
    root = desc;
}

libcxx::intrusive_ptr<filesystem::descriptor> process::get_cwd()
{
    return cwd;
}

libcxx::intrusive_ptr<filesystem::descriptor> process::get_root()
{
    return root;
}

void process::add_thread(thread* thread)
{
    if (threads.empty()) {
        // First node shares same PID
        thread->tid = this->pid;
    } else {
        // Rest of them increment the PID
        thread->tid = scheduler::get_free_pid();
    }
    threads.push_back(*thread);
}

void process::thread_exit(thread* thread, bool is_signal, int val)
{
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        auto& value = *it;
        if (&value == thread) {
            threads.erase(it);
            break;
        }
    }
    if (threads.empty()) {
        log::printk(log::log_level::DEBUG,
                    "Last thread exiting, process %d terminating\n", this->pid);
        this->exit(is_signal, val);
    }
}

process* process::fork()
{
    process* child = new process(this);
    scheduler::add_process(child);
    this->children.push_back(*child);
    addr_t cloned = memory::virt::fork();
    child->fds    = this->fds;
    child->set_root(this->get_root());
    child->set_cwd(this->get_cwd());
    child->vma           = new memory::vma(*this->vma);
    child->address_space = cloned;
    return child;
}

void process::exit(bool is_signal, int val)
{
    for (auto& section : *vma) {
        memory::virt::unmap_range(section.start(), section.size());
    }
    this->vma->reset();
    this->exit_reason = val;
    this->waiters.wakeup();
    // memory::physical::free(this->address_space);
    scheduler::remove_process(this->pid);
    delete this->vma;
}

int process::wait(int options)
{
    this->waiters.wait(scheduler::wait_interruptible);
    return this->exit_reason;
}

void process::send_signal(int signum)
{
    // TODO: Broadcast SIGSTOP, SIGCONT, certain signals to all threads
    // TODO: Randomly select a thread instead of the default (or maybe a
    // heuristic?)
    this->threads.front().send_signal(signum);
}
