#pragma once

#include <arch/proc/registers.h>
#include <lib/list.h>
#include <mm/section.h>
#include <proc/signal.h>
#include <types.h>

struct InterruptContext;

class Process;

enum ThreadState { SLEEPING, RUNNING };

class Thread
{
public:
    Thread(Process *p);
    ~Thread();
    bool load(addr_t binary, int argc, const char *argv[], int envc,
              const char *envp[], struct ThreadContext &ctx);
    void __attribute__((noreturn)) exit();

    tid_t tid;
    ThreadState state;
    struct ThreadContext cpu_ctx;  // Thread execution state
    addr_t kernel_stack;
    Node<Thread> process_node;
    Node<Thread> scheduler_node;
    Process *parent;

    // Signals
    size_t signal_count;
    bool signal_required;
    sigset_t signal_mask;
    sigset_t signal_pending;

    void handle_signal(struct InterruptContext *ctx);
    bool send_signal(int signal);
    void refresh_signal();
};

void save_context(struct InterruptContext *ctx,
                  struct ThreadContext *thread_ctx);
void load_context(struct InterruptContext *ctx,
                  struct ThreadContext *thread_ctx);

Thread *create_kernel_thread(Process *p, void (*entry_point)(void *),
                             void *data);

void set_stack(addr_t stack);
addr_t get_stack();

void set_thread_base(Thread *thread);

void load_registers(struct ThreadContext &cpu_ctx);
