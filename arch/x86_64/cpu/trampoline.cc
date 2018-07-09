#include <arch/cpu/registers.h>
#include <errno.h>
#include <kernel.h>
#include <proc/sched.h>
#include <proc/thread.h>

extern void* syscall_table[];

extern "C" void syscall_trampoline(struct InterruptContext* ctx)
{
    encode_tcontext(ctx, &Scheduler::get_current_thread()->tcontext);
    Log::printk(Log::LogLevel::DEBUG,
                "Received fast system call %d from PID %d, %lX %lX %lX "
                "%lX %lX\n",
                ctx->rax, Scheduler::get_current_thread()->tid, ctx->rdi,
                ctx->rsi, ctx->rdx, ctx->r10, ctx->r8);
    if (ctx->rax > 256 || !syscall_table[ctx->rax]) {
        Log::printk(Log::LogLevel::ERROR, "Received invalid syscall #%d\n", ctx->rax);
        ctx->rax = -ENOSYS;
        return;
    }
    uint64_t (*func)(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e) =
        (uint64_t(*)(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e))syscall_table[ctx->rax];
    ctx->rax = func(ctx->rdi, ctx->rsi, ctx->rdx, ctx->r10, ctx->r8);
    if (Scheduler::get_current_thread()->signal_required) {
        Scheduler::get_current_thread()->handle_signal(ctx);
    }
}
