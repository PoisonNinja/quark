#include <cpu/interrupt.h>
#include <proc/syscall.h>
#include <kernel.h>

namespace Syscall
{
static void handler(int, void*, struct interrupt_ctx* ctx) {
    Log::printk(Log::INFO, "Received system call %d\n", ctx->rax);
}

static struct Interrupt::Handler handler_data(handler, "syscall", &handler_data);

void init()
{
    Interrupt::register_handler(0x80, handler_data);
}
}
