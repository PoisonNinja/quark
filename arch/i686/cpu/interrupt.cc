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
    Log::printk(Log::ERROR, "EAX = %p EBX = %p ECX = %p EDX = %p\n", ctx->eax,
                ctx->ebx, ctx->ecx, ctx->edx);
    Log::printk(Log::ERROR, "ESI = %p EDI = %p EBP = %p ESP = %p\n", ctx->esi,
                ctx->edi, ctx->ebp, ctx->esp);
    Log::printk(Log::ERROR, "EIP = %p CS  = %p DS  = %p EFLAGS = %p\n",
                ctx->eip, ctx->cs, ctx->ds, ctx->eflags);
    Log::printk(Log::ERROR, "Exception #%d, error code 0x%X\n", ctx->int_no,
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
    return (ctx->cs == 0x1B);
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
