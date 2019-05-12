#include <arch/mm/layout.h>
#include <errno.h>
#include <kernel.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/wait.h>

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
    if (this->parent) {
        this->parent->notify_exit(this);
    }
    // memory::physical::free(this->address_space);
    delete this->vma;
    scheduler::remove_process(this->pid);
}

pid_t process::wait(pid_t pid, int* status, int options)
{
    if (children.empty()) {
        if (status)
            *status = -ECHILD;
        return -1;
    }
    // Verify that the target child exists
    if (pid > 0) {
        bool found = false;
        // Search through children
        for (auto& proc : children) {
            if (proc.pid == pid) {
                found = true;
            }
        }
        // Search through zombies
        for (auto& proc : zombies) {
            if (proc.pid == pid) {
                found = true;
            }
        }
        if (!found) {
            if (status)
                *status = -ECHILD;
            return -1;
        }
    }
    process* zombie = nullptr;
    while (1) {
        for (auto& z : zombies) {
            if (z.pid == pid || pid == -1) {
                zombie = &z;
                goto found;
            }
        }
        if (options & WNOHANG) {
            return 0;
        }
        int ret = this->waiters.wait(scheduler::wait_interruptible);
        if (ret) {
            if (status)
                *status = -EINTR;
            return -1;
        }
    }

found:
    if (status)
        *status = zombie->exit_reason;

    pid_t ret = zombie->pid;

    // TODO: Reap the zombie
    this->zombies.erase(this->zombies.iterator_to(*zombie));
    delete &zombie;

    return ret;
}

void process::notify_exit(process* child)
{
    this->waiters.wakeup();
    this->send_signal(SIGCHLD);
    // TODO: Do initial cleanup, such as adding into zombie queue
    this->children.erase(this->children.iterator_to(*child));
    this->zombies.push_front(*child);
}

void process::send_signal(int signum)
{
    // TODO: Broadcast SIGSTOP, SIGCONT, certain signals to all threads
    // TODO: Randomly select a thread instead of the default (or maybe a
    // heuristic?)
    this->threads.front().send_signal(signum);
}
