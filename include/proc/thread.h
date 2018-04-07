#pragma once

#include <arch/proc/registers.h>
#include <lib/list.h>
#include <mm/section.h>
#include <types.h>

struct InterruptContext;

class Process;

enum ThreadState { SLEEPING, RUNNING };

class Thread
{
public:
    Thread(Process *p);
    ~Thread();
    bool load(addr_t entry, int argc, const char *argv[], int envc,
              const char *envp[]);
    status_t save_context(struct InterruptContext *ctx);
    status_t load_context(struct InterruptContext *ctx);
    tid_t tid;
    ThreadState state;
    struct thread_ctx cpu_ctx;  // Thread execution state
    addr_t kernel_stack;
    Node<Thread> process_node;
    Node<Thread> scheduler_node;
    Process *parent;
};

void set_stack(addr_t stack);
addr_t get_stack();
