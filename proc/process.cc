#include <arch/mm/layout.h>
#include <kernel.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/process.h>
#include <proc/sched.h>

Process::Process(Process* parent)
{
    this->parent = parent;
    this->pid    = Scheduler::get_free_pid();
    this->vma    = new memory::vma(USER_START, USER_END);
    for (int i = 1; i < NSIGS; i++) {
        this->signal_actions[i].sa_handler = SIG_DFL;
        this->signal_actions[i].sa_flags   = 0;
    }
}

Process::~Process()
{
}

void Process::set_cwd(libcxx::intrusive_ptr<filesystem::Descriptor> desc)
{
    cwd = desc;
}

void Process::set_root(libcxx::intrusive_ptr<filesystem::Descriptor> desc)
{
    root = desc;
}

libcxx::intrusive_ptr<filesystem::Descriptor> Process::get_cwd()
{
    return cwd;
}

libcxx::intrusive_ptr<filesystem::Descriptor> Process::get_root()
{
    return root;
}

void Process::add_thread(Thread* thread)
{
    if (threads.empty()) {
        // First node shares same PID
        thread->tid = this->pid;
    } else {
        // Rest of them increment the PID
        thread->tid = Scheduler::get_free_pid();
    }
    threads.push_back(*thread);
}

void Process::remove_thread(Thread* thread)
{
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        auto& value = *it;
        if (&value == thread) {
            threads.erase(it);
            break;
        }
    }
    if (threads.empty()) {
        Log::printk(Log::LogLevel::DEBUG,
                    "Last thread exiting, process %d terminating\n", this->pid);
        this->exit();
    }
}

Process* Process::fork()
{
    Process* child = new Process(this);
    Scheduler::add_process(child);
    this->children.push_back(*child);
    addr_t cloned = memory::virt::fork();
    child->fds    = this->fds;
    child->set_root(this->get_root());
    child->set_cwd(this->get_cwd());
    child->vma           = new memory::vma(*this->vma);
    child->address_space = cloned;
    return child;
}

void Process::exit()
{
    for (auto& section : *vma) {
        memory::virt::unmap_range(section.start(), section.end());
    }
    this->vma->reset();
    memory::physical::free(this->address_space);
    Scheduler::remove_process(this->pid);
    delete this->vma;
}

void Process::send_signal(int signum)
{
    // TODO: Broadcast SIGSTOP, SIGCONT, certain signals to all threads
    // TODO: Randomly select a thread instead of the default (or maybe a
    // heuristic?)
    this->threads.front().send_signal(signum);
}
