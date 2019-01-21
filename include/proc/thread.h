#pragma once

#include <arch/proc/registers.h>
#include <lib/list.h>
#include <proc/signal.h>
#include <types.h>

struct interrupt_context;

class process;

enum class thread_state {
    UNMANAGED,
    SLEEPING_UNINTERRUPTIBLE,
    SLEEPING_INTERRUPTIBLE,
    RUNNABLE,
    RUNNING,
};

class thread
{
public:
    thread(process *p);
    ~thread();
    bool load(addr_t binary, int argc, const char *argv[], int envc,
              const char *envp[], struct thread_context &ctx);
    void exit();

    tid_t tid;
    thread_state state;
    struct thread_context tcontext; // Thread execution state
    libcxx::node<thread> process_node;
    libcxx::node<thread> scheduler_node;
    process *parent;

    // Signals
    size_t signal_count;
    bool signal_required;
    sigset_t signal_mask;
    sigset_t signal_pending;
    stack_t signal_stack;

    void handle_signal(struct interrupt_context *ctx);
    bool send_signal(int signal);

private:
    void setup_signal(struct ksignal *ksig,
                      struct thread_context *original_state,
                      struct thread_context *new_state);
    void refresh_signal();
};

void encode_tcontext(struct interrupt_context *ctx,
                     struct thread_context *thread_ctx);
void decode_tcontext(struct interrupt_context *ctx,
                     struct thread_context *thread_ctx);

void save_context(struct interrupt_context *ctx,
                  struct thread_context *tcontext);
void load_context(struct interrupt_context *ctx,
                  struct thread_context *tcontext);

thread *create_kernel_thread(process *p, void (*entry_point)(void *),
                             void *data);

void set_stack(addr_t stack);
addr_t get_stack();

void load_registers(struct thread_context &tcontext);
