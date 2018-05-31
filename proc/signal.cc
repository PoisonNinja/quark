#include <kernel.h>
#include <lib/string.h>
#include <proc/sched.h>
#include <proc/signal.h>
#include <proc/thread.h>

void Thread::handle_signal(struct InterruptContext* ctx)
{
    int signum = Signal::select_signal(&this->signal_pending);
    Log::printk(Log::DEBUG, "[signal]: Selecting signal %d\n", signum);

    // The signal is handled
    Signal::sigdelset(&this->signal_pending, signum);

    struct ThreadContext temp_state;
    save_context(ctx, &temp_state);

    struct sigaction* action = &this->parent->signal_actions[signum];

    // Figure out what our action is
    // TODO: Handle SIGSTOP, SIGCONT
    if (action->sa_handler == SIG_DFL) {
        Log::printk(Log::DEBUG, "[signal]: SIG_DFL, killing\n");
        this->exit();
    }

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
    // TODO: Actually check status of signals
    this->signal_required = true;
}

namespace Signal
{
void handle(struct InterruptContext* ctx)
{
    Scheduler::get_current_thread()->handle_signal(ctx);
}

int select_signal(sigset_t* set)
{
    if (sigismember(set, SIGKILL)) {
        return SIGKILL;
    } else if (sigismember(set, SIGSTOP)) {
        return SIGSTOP;
    } else {
        for (int i = 1; i < NSIGS; i++) {
            if (sigismember(set, i)) {
                return i;
            }
        }
    }
    return 0;
}

int sigemptyset(sigset_t* set)
{
    *set = 0;
    return 0;
}

int sigfillset(sigset_t* set)
{
    *set = 0xFFFFFFFF;
    return 0;
}

int sigaddset(sigset_t* set, int signum)
{
    if (signum <= 0 || signum > NSIGS) {
        return -1;
    }
    *set |= (1 << signum);
    return 0;
}

int sigdelset(sigset_t* set, int signum)
{
    if (signum <= 0 || signum > NSIGS) {
        return -1;
    }
    *set &= ~(1 << signum);
    return 0;
}

int sigismember(const sigset_t* set, int signum)
{
    if (signum <= 0 || signum > NSIGS) {
        return -1;
    }
    return (*set & (1 << signum)) ? 1 : 0;
}
}  // namespace Signal
