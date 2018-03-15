#include <cpu/interrupt.h>
#include <errno.h>
#include <kernel.h>
#include <proc/sched.h>
#include <proc/syscall.h>

namespace Syscall
{
static void sys_exit(int val)
{
    Scheduler::remove(Scheduler::get_current_thread());
    for (;;) {
        __asm__("hlt");
    }
}

static void* syscall_table[256];

static void handler(int, void*, struct interrupt_ctx* ctx)
{
    Log::printk(Log::DEBUG, "Received system call %d\n", ctx->rax);
    if (!syscall_table[ctx->rax]) {
        Log::printk(Log::ERROR, "Received invalid syscall #%d\n", ctx->rax);
        ctx->rax = -ENOSYS;
        return;
    }
    int (*func)(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) =
        (int (*)(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                 uint64_t e))syscall_table[ctx->rax];
    ctx->rax = func(ctx->rdi, ctx->rsi, ctx->rdx, ctx->rcx, ctx->r8);
}

static struct Interrupt::Handler handler_data(handler, "syscall",
                                              &handler_data);

void init()
{
    syscall_table[SYS_exit] = reinterpret_cast<void*>(sys_exit);
    Interrupt::register_handler(0x80, handler_data);
}
}
