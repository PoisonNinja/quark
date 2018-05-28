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

namespace Signal
{
void handle(struct InterruptContext* ctx)
{
    Scheduler::get_current_thread()->handle_signal(ctx);
}
}  // namespace Signal
