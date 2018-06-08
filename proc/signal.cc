#include <kernel.h>
#include <lib/string.h>
#include <proc/sched.h>
#include <proc/signal.h>
#include <proc/thread.h>

static sigset_t ignored_signals;
static sigset_t stop_signals;
static sigset_t unblockable_signals;

void Thread::handle_signal(struct InterruptContext* ctx)
{
    sigset_t unblocked_signals;
    Signal::signotset(&unblocked_signals, &this->signal_mask);
    // Ensure that SIGKILL and SIGSTOP are always unblocked
    Signal::sigorset(&unblocked_signals, &unblockable_signals);

    sigset_t deliverable_signals;
    Signal::sigemptyset(&deliverable_signals);
    Signal::sigorset(&deliverable_signals, &unblocked_signals);
    Signal::sigandset(&deliverable_signals, &this->signal_pending);

    int signum = Signal::select_signal(&deliverable_signals);
    if (!signum) {
        return;
    }
    Log::printk(Log::DEBUG, "[signal]: Selecting signal %d\n", signum);

    // The signal is handled
    Signal::sigdelset(&this->signal_pending, signum);
    this->refresh_signal();

    if (signum == SIGKILL) {
        Log::printk(Log::DEBUG, "[signal]: SIGKILL, killing\n");
        this->exit();
    }

    struct sigaction* action = &this->parent->signal_actions[signum];

    if (action->sa_handler == SIG_IGN) {
        Log::printk(Log::DEBUG, "[signal]: SIG_IGN, returning\n");
        return;
    }

    if ((Signal::sigismember(&stop_signals, signum) &&
         action->sa_handler == SIG_DFL) ||
        signum == SIGSTOP) {
        Log::printk(Log::WARNING,
                    "[signal]: SIG_DFL with SIGSTOP, you "
                    "should probably implement it. Killing for now\n");
        this->exit();
    }

    if (signum == SIGCONT) {
        Log::printk(Log::WARNING,
                    "[signal]: SIGCONT, you "
                    "should probably implement it. Killing for now\n");
        this->exit();
    }

    if (action->sa_handler == SIG_DFL) {
        Log::printk(
            Log::DEBUG,
            "[signal]: SIG_DFL with default action of SIGKILl, killing\n");
        this->exit();
    }

    Log::printk(Log::DEBUG, "[signal] Has a sa_handler, calling\n");

    bool requested_altstack = action->sa_flags & SA_ONSTACK;
    bool usable_altstack =
        !(this->signal_stack.ss_flags & (SS_ONSTACK | SS_DISABLE));
    bool use_altstack = requested_altstack && usable_altstack;

    if (use_altstack) {
        Log::printk(Log::DEBUG, "[signal] Signal handler requests and has a "
                                "valid alternate stack, using...\n");
        this->signal_stack.ss_flags |= SS_ONSTACK;
    }

    struct ThreadContext new_state, original_state;
    save_context(ctx, &original_state);

    siginfo_t siginfo;
    siginfo.si_signo = signum;

    ucontext_t ucontext = {
        .uc_link = nullptr,
        .uc_sigmask = this->signal_mask,
    };

    String::memcpy(&ucontext.uc_stack, &this->signal_stack,
                   sizeof(this->signal_stack));
    Signal::encode_mcontext(&ucontext.uc_mcontext, &original_state);

    struct ksignal ksig = {
        .signum = signum,
        .use_altstack = use_altstack,
        .sa = action,
        .siginfo = &siginfo,
        .ucontext = &ucontext,
    };

    this->setup_signal(&ksig, &original_state, &new_state);

    load_context(ctx, &new_state);
    return;
}

bool Thread::send_signal(int signum)
{
    if (signum == 0) {
        return false;
    }

    if (signum < 0 || signum >= NSIGS) {
        return false;
    }
    // TODO: Check if signal is pending already and return ESIGPENDING
    Signal::sigaddset(&this->signal_pending, signum);
    this->refresh_signal();
    return true;
}

void Thread::refresh_signal()
{
    // TODO: Actually check status of signals
    sigset_t possible;
    Signal::sigemptyset(&possible);
    Signal::sigorset(&possible, &signal_pending);

    sigset_t unblocked_signals;
    Signal::signotset(&unblocked_signals, &this->signal_mask);
    // Ensure that SIGKILL and SIGSTOP are always unblocked
    Signal::sigorset(&unblocked_signals, &unblockable_signals);

    Signal::sigandset(&possible, &unblocked_signals);

    // TODO: Extend heuristics to ignore actions that would result in the
    // signal being ignored (e.g. SIG_DFL with default ignored or SIG_IGN)
    this->signal_required = !Signal::sigisemptyset(&possible);
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
    if (signum <= 0 || signum >= NSIGS) {
        return -1;
    }
    *set |= (1 << signum);
    return 0;
}

int sigdelset(sigset_t* set, int signum)
{
    if (signum <= 0 || signum >= NSIGS) {
        return -1;
    }
    *set &= ~(1 << signum);
    return 0;
}

int sigismember(const sigset_t* set, int signum)
{
    if (signum <= 0 || signum >= NSIGS) {
        return -1;
    }
    return (*set & (1 << signum)) ? 1 : 0;
}

bool sigisemptyset(sigset_t* set)
{
    return (*set) ? false : true;
}

void sigandset(sigset_t* dest, const sigset_t* source)
{
    *dest &= *source;
}

void sigorset(sigset_t* dest, const sigset_t* source)
{
    *dest |= *source;
}

void signotset(sigset_t* dest, const sigset_t* source)
{
    *dest = ~*source;
}

void init()
{
    sigemptyset(&ignored_signals);
    sigaddset(&ignored_signals, SIGCHLD);
    sigaddset(&ignored_signals, SIGURG);
    sigaddset(&ignored_signals, SIGPWR);
    sigaddset(&ignored_signals, SIGWINCH);
    sigemptyset(&stop_signals);
    sigaddset(&stop_signals, SIGTSTP);
    sigaddset(&stop_signals, SIGTTIN);
    sigaddset(&stop_signals, SIGTTOU);
    sigemptyset(&unblockable_signals);
    sigaddset(&unblockable_signals, SIGKILL);
    sigaddset(&unblockable_signals, SIGSTOP);
}
}  // namespace Signal
