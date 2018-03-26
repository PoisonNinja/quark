#pragma once

#include <fs/descriptor.h>
#include <fs/dtable.h>
#include <lib/list.h>
#include <proc/thread.h>
#include <types.h>

class Process
{
public:
    Process(Process* parent);
    ~Process();
    pid_t pid;
    addr_t address_space;
    status_t add_thread(Thread* thread);
    status_t remove_thread(Thread* thread);
    Ref<Filesystem::Descriptor> cwd;
    Ref<Filesystem::Descriptor> root;
    Filesystem::DTable fds;
    Memory::SectionManager* sections;

private:
    Process *parent, *children;
    List<Thread, &Thread::process_node> threads;
};
