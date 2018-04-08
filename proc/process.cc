#include <arch/mm/layout.h>
#include <mm/virtual.h>
#include <proc/process.h>
#include <proc/sched.h>

Process::Process(Process* parent)
{
    this->parent = parent;
    this->pid = Scheduler::get_free_pid();
    this->sections = new Memory::SectionManager(USER_START, USER_END);
}

Process::~Process()
{
}

void Process::set_cwd(Ref<Filesystem::Descriptor> desc)
{
    cwd = desc;
}

void Process::set_root(Ref<Filesystem::Descriptor> desc)
{
    root = desc;
}

void Process::set_dtable(Ref<Filesystem::DTable> table)
{
    fds = table;
}

Ref<Filesystem::Descriptor> Process::get_cwd()
{
    return cwd;
}

Ref<Filesystem::Descriptor> Process::get_root()
{
    return root;
}

Ref<Filesystem::DTable> Process::get_dtable()
{
    return fds;
}

bool Process::add_thread(Thread* thread)
{
    if (threads.empty()) {
        // First node shares same PID
        thread->tid = this->pid;
    } else {
        // Rest of them increment the PID
        thread->tid = Scheduler::get_free_pid();
    }
    threads.push_back(*thread);
    return true;
}

bool Process::remove_thread(Thread* thread)
{
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        auto& value = *it;
        if (&value == thread) {
            threads.erase(it);
            return true;
        }
    }
    return false;
}

Process* Process::fork()
{
    Process* child = new Process(this);
    addr_t cloned = Memory::Virtual::fork();
    child->set_dtable(
        Ref<Filesystem::DTable>(new Filesystem::DTable(*this->fds)));
    child->set_root(Scheduler::get_current_process()->get_root());
    child->set_cwd(Scheduler::get_current_process()->get_cwd());
    child->sections = new Memory::SectionManager(*this->sections);
    child->address_space = cloned;
    return child;
}
