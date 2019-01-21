#include <arch/cpu/registers.h>
#include <cpu/interrupt.h>
#include <drivers/irqchip/intel-8259.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>

namespace interrupt
{
void disable(void)
{
    __asm__("cli");
}

void enable(void)
{
    __asm__("sti");
}

void dump(interrupt_context* ctx)
{
#ifdef X86_64
    log::printk(log::log_level::ERROR, "RAX = %p RBX = %p RCX = %p RDX = %p\n",
                ctx->rax, ctx->rbx, ctx->rcx, ctx->rdx);
    log::printk(log::log_level::ERROR, "RSI = %p RDI = %p RBP = %p RSP = %p\n",
                ctx->rsi, ctx->rdi, ctx->rbp, ctx->rsp);
    log::printk(log::log_level::ERROR, "R8  = %p R9  = %p R10 = %p R11 = %p\n",
                ctx->r8, ctx->r9, ctx->r10, ctx->r11);
    log::printk(log::log_level::ERROR, "R12 = %p R13 = %p R14 = %p R15 = %p\n",
                ctx->r12, ctx->r13, ctx->r14, ctx->r15);
    log::printk(log::log_level::ERROR,
                "RIP = %p CS  = %p DS  = %p RFLAGS = %p\n", ctx->rip, ctx->cs,
                ctx->ds, ctx->rflags);
#else
    log::printk(log::log_level::ERROR, "EAX = %p EBX = %p ECX = %p EDX = %p\n",
                ctx->eax, ctx->ebx, ctx->ecx, ctx->edx);
    log::printk(log::log_level::ERROR, "ESI = %p EDI = %p EBP = %p ESP = %p\n",
                ctx->esi, ctx->edi, ctx->ebp, ctx->esp);
    log::printk(log::log_level::ERROR,
                "EIP = %p CS  = %p DS  = %p EFLAGS = %p\n", ctx->eip, ctx->cs,
                ctx->ds, ctx->eflags);
#endif
    log::printk(log::log_level::ERROR, "Exception #%d, error code 0x%X\n",
                ctx->int_no, ctx->err_code);
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

bool is_userspace(struct interrupt_context* ctx)
{
    // The CS values being compared to are the selectors for user code segment
#ifdef X86_64
    return (ctx->cs == 0x23);
#else
    return (ctx->cs == 0x1B);
#endif
}

bool is_exception(int int_no)
{
    // x86 IRQs start at 32
    if (int_no < 32) {
        return true;
    }
    return false;
}

void arch_init(void)
{
    irqchip::intel8259* pic = new irqchip::intel8259();
    irqchip::set_irqchip(*pic);
}

extern "C" void arch_handler(struct interrupt_context* ctx)
{
    interrupt::dispatch(ctx->int_no, ctx);
}
}; // namespace interrupt
