#pragma once

#include <arch/proc/registers.h>
#include <lib/list.h>
#include <types.h>

class Process;

enum ThreadState {};

class Thread
{
public:
    Thread(Process *p) : parent(p){};
    ~Thread();
    ThreadState state;
    struct thread_ctx cpu_ctx;
    Node<Thread> process_node;
    Node<Thread> scheduler_node;
    Process *parent;
};
