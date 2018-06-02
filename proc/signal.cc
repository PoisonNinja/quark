#include <arch/cpu/registers.h>
#include <kernel.h>
#include <lib/string.h>
#include <proc/sched.h>
#include <proc/signal.h>
#include <proc/thread.h>

struct stack_frame {
    uint64_t ret_location;
    InterruptContext ctx;
};

void Thread::handle_signal(struct InterruptContext* ctx)
{
    int signum = Signal::select_signal(&this->signal_pending);
    Log::printk(Log::DEBUG, "[signal]: Selecting signal %d\n", signum);

    // The signal is handled
    Signal::sigdelset(&this->signal_pending, signum);

    struct sigaction* action = &this->parent->signal_actions[signum];

    // Figure out what our action is
    // TODO: Handle SIGSTOP, SIGCONT
    if (action->sa_handler == SIG_DFL) {
        Log::printk(Log::DEBUG, "[signal]: SIG_DFL, killing\n");
        this->exit();
    }

    Log::printk(Log::DEBUG, "[signal] Has a sa_handler, calling\n");

    struct ThreadContext temp_state, original_state;
    save_context(ctx, &temp_state);
    save_context(ctx, &original_state);

    // KLUDGE! TODO: Make this architecture independent
    // Construct a stack return frame

    temp_state.rsp -= 128;
    temp_state.rsp -= sizeof(struct stack_frame);
    temp_state.rsp &= ~(16UL - 1UL);
    struct stack_frame* frame = (struct stack_frame*)temp_state.rsp;

    String::memcpy(&frame->ctx, ctx, sizeof(*ctx));
    frame->ret_location = this->parent->sigreturn;

    Log::printk(Log::INFO, "Going to return to gadget at %p\n",
                frame->ret_location);
    Log::printk(Log::INFO, "Frame at %p\n", frame);

    temp_state.rip = (uint64_t)action->sa_handler;
    temp_state.rdi = signum;
    temp_state.rsi = 0;
    temp_state.rsp = (uint64_t)frame;

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
