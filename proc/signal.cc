#include <lib/string.h>
#include <proc/sched.h>
#include <proc/signal.h>
#include <proc/thread.h>

void Thread::handle_signal(struct InterruptContext* ctx)
{
    struct ThreadContext temp_state;
    save_context(ctx, &temp_state);
    temp_state.rip = 0x1000;
    load_context(ctx, &temp_state);
    return;
}

bool Thread::send_signal(int signum)
{
    // TODO: Check if signal is pending already and return ESIGPENDING
    Signal::sigaddset(&this->signal_pending, signum);
    this->refresh_signal();
}

void Thread::refresh_signal()
{
}

namespace Signal
{
void handle(struct InterruptContext* ctx)
{
    Scheduler::get_current_thread()->handle_signal(ctx);
}

int sigemptyset(sigset_t* set)
{
    String::memset(set, 0, sizeof(*set));
    return 0;
}

int sigfillset(sigset_t* set)
{
    String::memset(set, 0xFFFF, sizeof(*set));
    return 0;
}

int sigaddset(sigset_t* set, int signum)
{
    if (signum <= 0 || signum > NSIGS) {
        return -1;
    }
    set->sigs[signum % 8] |= (1 << signum & 8);
    return 0;
}

int sigdelset(sigset_t* set, int signum)
{
    if (signum <= 0 || signum > NSIGS) {
        return -1;
    }
    set->sigs[signum % 8] &= ~(1 << signum & 8);
    return 0;
}

int sigismember(const sigset_t* set, int signum)
{
    if (signum <= 0 || signum > NSIGS) {
        return -1;
    }
    return (set->sigs[signum % 8] & (1 << signum & 8)) ? 1 : 0;
}
}  // namespace Signal
