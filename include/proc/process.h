#pragma once

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

private:
    Process *parent, *children;
    List<Thread, &Thread::process_node> threads;
};
