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
    thread(process *p, tid_t tid);
    ~thread();
    void exit(bool is_signal, int val);

    thread_state state;
    struct thread_context tcontext; // Thread execution state

    tid_t get_tid();
    process *get_process();

    libcxx::node<thread> process_node;
    libcxx::node<thread> scheduler_node;

    void load_state(interrupt_context *ctx);
    void save_state(interrupt_context *ctx);

    // Don't you just love signals?
    bool is_signal_pending();
    void handle_signal(struct interrupt_context *ctx);
    void handle_sigreturn(ucontext_t *uctx);
    int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
    int sigpending(sigset_t *set);
    int sigaltstack(const stack_t *ss, stack_t *oldss);
    bool send_signal(int signal);

private:
    tid_t tid;
    process *parent;

    // Signals
    size_t signal_count;
    bool signal_required;
    sigset_t signal_mask;
    sigset_t signal_pending;
    stack_t signal_stack;

    void setup_signal(struct ksignal *ksig,
                      struct thread_context *original_state,
                      struct thread_context *new_state);
    void refresh_signal();
};

void encode_tcontext(struct interrupt_context *ctx,
                     struct thread_context *thread_ctx);
void decode_tcontext(struct interrupt_context *ctx,
                     struct thread_context *thread_ctx);

thread *create_kernel_thread(process *p, void (*entry_point)(void *),
                             void *data);

void set_stack(addr_t stack);
addr_t get_stack();

void load_registers(struct thread_context &tcontext);
