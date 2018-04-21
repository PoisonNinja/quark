#include <arch/mm/layout.h>
#include <kernel.h>
#include <mm/physical.h>
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
        Log::printk(Log::DEBUG, "Last thread exiting, process %d terminating\n",
                    this->pid);
        this->exit();
    }
}

Process* Process::fork()
{
    Process* child = new Process(this);
    this->children.push_back(*child);
    addr_t cloned = Memory::Virtual::fork();
    child->set_dtable(
        Ref<Filesystem::DTable>(new Filesystem::DTable(*this->fds)));
    child->set_root(this->get_root());
    child->set_cwd(this->get_cwd());
    child->sections = new Memory::SectionManager(*this->sections);
    child->address_space = cloned;
    return child;
}

void Process::exit()
{
    for (auto section : *sections) {
        Memory::Virtual::unmap_range(section.start(), section.end());
    }
    Memory::Physical::free(this->address_space);
    delete this->sections;
}
