#include <arch/cpu/syscall.h>
#include <cpu/interrupt.h>
#include <errno.h>
#include <kernel.h>
#include <proc/sched.h>
#include <proc/thread.h>

extern void* syscall_table[];
namespace CPU
{
namespace X86
{
static void syscall_handler(int, void*, struct InterruptContext* ctx)
{
    save_context(ctx, &Scheduler::get_current_thread()->cpu_ctx);
    Log::printk(Log::DEBUG,
                "Received legacy system call %d from PID %d, %lX %lX %lX "
                "%lX %lX\n",
                ctx->eax, Scheduler::get_current_thread()->tid, ctx->ebx,
                ctx->ecx, ctx->edx, ctx->edi, ctx->esi);
    if (ctx->eax > 256 || !syscall_table[ctx->eax]) {
        Log::printk(Log::ERROR, "Received invalid syscall #%d\n", ctx->eax);
        ctx->eax = -ENOSYS;
        return;
    }
    uint32_t (*func)(uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                     uint32_t e) =
        (uint32_t(*)(uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                     uint32_t e))syscall_table[ctx->eax];
    ctx->eax = func(ctx->ebx, ctx->ecx, ctx->edx, ctx->edi, ctx->esi);
    if (Scheduler::get_current_thread()->signal_required) {
        Scheduler::get_current_thread()->handle_signal(ctx);
    }
}

static struct Interrupt::Handler syscall_handler_data(syscall_handler,
                                                      "syscall",
                                                      &syscall_handler_data);

void init_syscalls()
{
    Interrupt::register_handler(0x80, syscall_handler_data);
}
}  // namespace X86
}  // namespace CPU
