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
    tid_t tid;
    ThreadState state;
    struct ThreadContext cpu_ctx;  // Thread execution state
    addr_t kernel_stack;
    Node<Thread> process_node;
    Node<Thread> scheduler_node;
    Process *parent;
};

void save_context(struct InterruptContext *ctx,
                  struct ThreadContext *thread_ctx);
void load_context(struct InterruptContext *ctx,
                  struct ThreadContext *thread_ctx);

Thread *create_kernel_thread(Process *p, void (*entry_point)(void *),
                             void *data);

void set_stack(addr_t stack);
addr_t get_stack();
