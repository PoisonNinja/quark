#pragma once

#include <arch/proc/registers.h>
#include <lib/list.h>
#include <types.h>

struct interrupt_ctx;

class Process;

enum ThreadState { SLEEPING, RUNNING };

class Thread
{
public:
    Thread(Process *p);
    ~Thread();
    tid_t tid;
    ThreadState state;
    struct thread_ctx cpu_ctx;
    addr_t kernel_stack;
    Node<Thread> process_node;
    Node<Thread> scheduler_node;
    Process *parent;
};

status_t save_context(Thread *thread, struct interrupt_ctx *ctx);
status_t load_context(Thread *thread, struct interrupt_ctx *ctx);

void set_stack(addr_t stack);
addr_t get_stack();
