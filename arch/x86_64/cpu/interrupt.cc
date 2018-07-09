#include <arch/cpu/registers.h>
#include <cpu/interrupt.h>
#include <drivers/irqchip/intel-8259.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>

namespace Interrupt
{
void disable(void)
{
    __asm__("cli");
}

void enable(void)
{
    __asm__("sti");
}

void dump(InterruptContext* ctx)
{
    Log::printk(Log::LogLevel::ERROR, "RAX = %p RBX = %p RCX = %p RDX = %p\n", ctx->rax,
                ctx->rbx, ctx->rcx, ctx->rdx);
    Log::printk(Log::LogLevel::ERROR, "RSI = %p RDI = %p RBP = %p RSP = %p\n", ctx->rsi,
                ctx->rdi, ctx->rbp, ctx->rsp);
    Log::printk(Log::LogLevel::ERROR, "R8  = %p R9  = %p R10 = %p R11 = %p\n", ctx->r8,
                ctx->r9, ctx->r10, ctx->r11);
    Log::printk(Log::LogLevel::ERROR, "R12 = %p R13 = %p R14 = %p R15 = %p\n", ctx->r12,
                ctx->r13, ctx->r14, ctx->r15);
    Log::printk(Log::LogLevel::ERROR, "RIP = %p CS  = %p DS  = %p RFLAGS = %p\n",
                ctx->rip, ctx->cs, ctx->ds, ctx->rflags);
    Log::printk(Log::LogLevel::ERROR, "Exception #%d, error code 0x%X\n", ctx->int_no,
                ctx->err_code);
}

bool interrupts_enabled(void)
{
    unsigned long eflags;
    asm volatile("pushf\n\t"
                 "pop %0"
                 : "=rm"(eflags)
                 :
                 : "memory");
    return eflags & (1 << 9);
}

bool is_userspace(struct InterruptContext* ctx)
{
    return (ctx->cs == 0x23);
}

bool is_exception(int int_no)
{
    if (int_no < 32) {
        return true;
    }
    return false;
}

void arch_init(void)
{
    IrqChip::Intel8259* pic = new IrqChip::Intel8259();
    IrqChip::set_irqchip(*pic);
}

extern "C" void arch_handler(struct InterruptContext* ctx)
{
    Interrupt::dispatch(ctx->int_no, ctx);
}
};  // namespace Interrupt
