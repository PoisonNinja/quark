#include <arch/cpu/registers.h>
#include <cpu/interrupt.h>
#include <errno.h>
#include <kernel.h>
#include <proc/sched.h>
#include <proc/thread.h>

extern void* syscall_table[];

#ifdef X86_64
extern "C" void syscall_trampoline(struct interrupt_context* ctx)
{
    encode_tcontext(ctx, &scheduler::get_current_thread()->tcontext);
    log::printk(log::log_level::DEBUG,
                "Received fast system call %d from PID %d, %lX %lX %lX "
                "%lX %lX\n",
                ctx->rax, scheduler::get_current_thread()->tid, ctx->rdi,
                ctx->rsi, ctx->rdx, ctx->r10, ctx->r8);
    if (ctx->rax > 256 || !syscall_table[ctx->rax]) {
        log::printk(log::log_level::ERROR, "Received invalid syscall #%d\n",
                    ctx->rax);
        ctx->rax = -ENOSYS;
        return;
    }
    uint64_t (*func)(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e) =
        (uint64_t(*)(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e))syscall_table[ctx->rax];
    ctx->rax = func(ctx->rdi, ctx->rsi, ctx->rdx, ctx->r10, ctx->r8);
    if (scheduler::get_current_thread()->signal_required) {
        scheduler::get_current_thread()->handle_signal(ctx);
    }
}
#else
namespace cpu
{
namespace x86
{
static void syscall_handler(int, void*, struct interrupt_context* ctx)
{
    encode_tcontext(ctx, &scheduler::get_current_thread()->tcontext);
    log::printk(log::log_level::DEBUG,
                "Received legacy system call %d from PID %d, %lX %lX %lX "
                "%lX %lX\n",
                ctx->eax, scheduler::get_current_thread()->tid, ctx->ebx,
                ctx->ecx, ctx->edx, ctx->edi, ctx->esi);
    if (ctx->eax > 256 || !syscall_table[ctx->eax]) {
        log::printk(log::log_level::ERROR, "Received invalid syscall #%d\n",
                    ctx->eax);
        ctx->eax = -ENOSYS;
        return;
    }
    uint32_t (*func)(uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                     uint32_t e) =
        (uint32_t(*)(uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                     uint32_t e))syscall_table[ctx->eax];
    ctx->eax = func(ctx->ebx, ctx->ecx, ctx->edx, ctx->edi, ctx->esi);
    if (scheduler::get_current_thread()->signal_required) {
        scheduler::get_current_thread()->handle_signal(ctx);
    }
}

static struct interrupt::handler
    syscall_handler_data(syscall_handler, "syscall", &syscall_handler_data);

void init_syscalls()
{
    interrupt::register_handler(0x80, syscall_handler_data);
}
} // namespace x86
} // namespace cpu
#endif
