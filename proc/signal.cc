#include <errno.h>
#include <kernel.h>
#include <lib/string.h>
#include <proc/sched.h>
#include <proc/signal.h>
#include <proc/thread.h>

static sigset_t ignored_signals;
static sigset_t stop_signals;
static sigset_t unblockable_signals;

void thread::handle_signal(struct interrupt_context* ctx)
{
    sigset_t unblocked_signals;
    signal::signotset(&unblocked_signals, &this->signal_mask);
    // Ensure that SIGKILL and SIGSTOP are always unblocked
    signal::sigorset(&unblocked_signals, &unblockable_signals);

    sigset_t deliverable_signals;
    signal::sigemptyset(&deliverable_signals);
    signal::sigorset(&deliverable_signals, &unblocked_signals);
    signal::sigandset(&deliverable_signals, &this->signal_pending);

    int signum = signal::select_signal(&deliverable_signals);
    if (!signum) {
        return;
    }
    log::printk(log::log_level::DEBUG, "[signal]: Selecting signal %d\n",
                signum);

    // The signal is handled
    signal::sigdelset(&this->signal_pending, signum);
    this->refresh_signal();

    if (signum == SIGKILL) {
        log::printk(log::log_level::DEBUG, "[signal]: SIGKILL, killing\n");
        this->exit(true, signum);
    }

    struct sigaction* action = &this->parent->signal_actions[signum];

    if (action->sa_handler == SIG_IGN ||
        (action->sa_handler == SIG_DFL &&
         signal::sigismember(&ignored_signals, signum))) {
        log::printk(log::log_level::DEBUG, "[signal]: SIG_IGN, returning\n");
        return;
    }

    if ((signal::sigismember(&stop_signals, signum) &&
         action->sa_handler == SIG_DFL) ||
        signum == SIGSTOP) {
        log::printk(log::log_level::WARNING,
                    "[signal]: SIG_DFL with SIGSTOP, you "
                    "should probably implement it. Killing for now\n");
        this->exit(true, SIGSTOP);
    }

    if (signum == SIGCONT) {
        log::printk(log::log_level::WARNING,
                    "[signal]: SIGCONT, you "
                    "should probably implement it. Killing for now\n");
        this->exit(true, SIGCONT);
    }

    if (action->sa_handler == SIG_DFL) {
        log::printk(
            log::log_level::DEBUG,
            "[signal]: SIG_DFL with default action of SIGKILl, killing\n");
        this->exit(true, SIGKILL);
    }

    log::printk(log::log_level::DEBUG, "[signal] Has a sa_handler, calling\n");

    bool requested_altstack = action->sa_flags & SA_ONSTACK;
    bool usable_altstack =
        !(this->signal_stack.ss_flags & (SS_ONSTACK | SS_DISABLE));
    bool use_altstack = requested_altstack && usable_altstack;

    if (use_altstack) {
        log::printk(log::log_level::DEBUG,
                    "[signal] signal handler requests and has a "
                    "valid alternate stack, using...\n");
        this->signal_stack.ss_flags |= SS_ONSTACK;
    }

    struct thread_context new_state, original_state;
    encode_tcontext(ctx, &original_state);

    siginfo_t siginfo;
    siginfo.si_signo = signum;

    ucontext_t ucontext = {
        .uc_link    = nullptr,
        .uc_sigmask = this->signal_mask,
    };

    libcxx::memcpy(&ucontext.uc_stack, &this->signal_stack,
                   sizeof(this->signal_stack));
    signal::encode_mcontext(&ucontext.uc_mcontext, &original_state);

    struct ksignal ksig = {
        .signum       = signum,
        .use_altstack = use_altstack,
        .sa           = action,
        .siginfo      = &siginfo,
        .ucontext     = &ucontext,
    };

    // Mask out current signal if SA_NODEFER is not passed in
    if (!(action->sa_flags & SA_NODEFER)) {
        signal::sigaddset(&this->signal_mask, signum);
    }

    this->setup_signal(&ksig, &original_state, &new_state);

    decode_tcontext(ctx, &new_state);
    return;
}

bool thread::send_signal(int signum)
{
    if (signum == 0) {
        return false;
    }

    if (signum < 0 || signum >= NSIGS) {
        return false;
    }
    // TODO: Check if signal is pending already and return ESIGPENDING
    signal::sigaddset(&this->signal_pending, signum);
    this->refresh_signal();
    if (this->is_signal_pending() &&
        this->state == thread_state::SLEEPING_INTERRUPTIBLE) {
        scheduler::insert(this);
    }
    return true;
}

void thread::handle_sigreturn(ucontext_t* uctx)
{
    thread_context tctx;
    /*
     * The syscall handler saves the userspace context, so we copy it into
     * tctx to get certain registers (DS, ES, SS) preloaded for us. The
     * rest of the state will get overriden by the stored mcontext
     */
    libcxx::memcpy(&tctx, &this->tcontext, sizeof(tctx));
    // Unset on_stack
    if (this->signal_stack.ss_flags & SS_ONSTACK) {
        this->signal_stack.ss_flags &= ~SS_ONSTACK;
    }
    // Restore signal mask
    this->signal_mask = uctx->uc_sigmask;
    signal::decode_mcontext(&uctx->uc_mcontext, &tctx);
    load_registers(tctx);
}

int thread::sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
    if (oldset) {
        *oldset = this->signal_mask;
    }
    if (how == SIG_SETMASK) {
        this->signal_mask = *set;
    } else if (how == SIG_BLOCK) {
        signal::sigorset(&this->signal_mask, set);
    } else if (how == SIG_UNBLOCK) {
        sigset_t dup = *set;
        signal::signotset(&dup, &dup);
        signal::sigandset(&this->signal_mask, &dup);
    } else {
        return -EINVAL;
    }
    return 0;
}

int thread::sigpending(sigset_t* set)
{
    if (!set) {
        return -EFAULT;
    }
    sigset_t pending = scheduler::get_current_thread()->signal_pending;
    /*
     * Signals that are both blocked and ignored are NOT added to the mask
     * of pending signals.
     *
     * However, signals that are merely blocked will be added
     */
    sigset_t blocked_and_ignored;
    // Get the list of blocked signals
    signal::sigorset(&blocked_and_ignored,
                     &scheduler::get_current_thread()->signal_mask);
    // Remove signals that are NOT ignored
    for (int i = 1; i < NSIGS; i++) {
        // TODO: Also check if SIG_DFL is set but the default action is to
        // ignore
        if (scheduler::get_current_process()->signal_actions[i].sa_handler !=
            SIG_IGN) {
            signal::sigdelset(&blocked_and_ignored, i);
        }
    }
    // Invert the set so that signals that are both blocked and ignored are
    // unset
    signal::signotset(&blocked_and_ignored, &blocked_and_ignored);
    // AND the two sets together to basically unset blocked and ignored
    signal::sigandset(&pending, &blocked_and_ignored);

    *set = pending;

    /*
     * TODO: Figure out what happens if a signal is handled right after this
     * system call returns.
     *
     * System calls will check for pending signals before returning and will
     * handle them before returning so it's possible that the set returned
     * still has the bit set even though the signal is already handled.
     */
    return 0;
}

int thread::sigaltstack(const stack_t* ss, stack_t* oldss)
{
    // TODO: EFAULT?
    if (oldss) {
        libcxx::memcpy(oldss, &scheduler::get_current_thread()->signal_stack,
                       sizeof(*oldss));
    }
    if (ss) {
        libcxx::memcpy(&scheduler::get_current_thread()->signal_stack, ss,
                       sizeof(*ss));
    }
    return 0;
}

void thread::refresh_signal()
{
    // TODO: Actually check status of signals
    sigset_t possible;
    signal::sigemptyset(&possible);
    signal::sigorset(&possible, &signal_pending);

    sigset_t unblocked_signals;
    signal::signotset(&unblocked_signals, &this->signal_mask);
    // Ensure that SIGKILL and SIGSTOP are always unblocked
    signal::sigorset(&unblocked_signals, &unblockable_signals);

    signal::sigandset(&possible, &unblocked_signals);

    // TODO: Extend heuristics to ignore actions that would result in the
    // signal being ignored (e.g. SIG_DFL with default ignored or SIG_IGN)
    this->signal_required = !signal::sigisemptyset(&possible);
}

bool thread::is_signal_pending()
{
    return this->signal_required;
}

namespace signal
{
void handle(struct interrupt_context* ctx)
{
    scheduler::get_current_thread()->handle_signal(ctx);
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
} // namespace signal
