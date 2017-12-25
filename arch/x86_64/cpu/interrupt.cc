#include <arch/cpu/registers.h>
#include <cpu/interrupt.h>
#include <drivers/irqchip/intel-8259.h>
#include <drivers/irqchip/irqchip.h>

#include <kernel.h>

namespace Interrupt
{
void arch_disable(void)
{
    __asm__("cli");
}

void arch_enable(void)
{
    __asm__("sti");
}

void arch_init(void)
{
    IrqChip::Intel8259* pic = new IrqChip::Intel8259();
    Log::printk(Log::INFO, "PIC at %p\n", pic);
    IrqChip::set_irqchip(*pic);
}

extern "C" void arch_handler(struct interrupt_ctx* ctx)
{
    Interrupt::dispatch(ctx->int_no, ctx);
}
};  // namespace Interrupt
