#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>
#include <atomic>

namespace Interrupt
{
static List<Interrupt::Handler, &Interrupt::Handler::node> handlers[max];

static std::atomic<int> interrupt_depth(1);

extern void arch_disable();
extern void arch_enable();
extern void arch_init();
extern bool arch_interrupt_enabled();

int disable()
{
    if (!atomic_fetch_add(&interrupt_depth, 1))
        Interrupt::arch_disable();
    return interrupt_depth;
}

int enable()
{
    if (atomic_fetch_sub(&interrupt_depth, 1) == 1)
        Interrupt::arch_enable();
    return interrupt_depth;
}

void save(int& store)
{
    store = arch_interrupt_enabled();
}

void restore(int& store)
{
    if (store) {
        enable();
    } else {
        disable();
    }
 }

void dispatch(int int_no, struct InterruptContext* ctx)
{
    if (handlers[int_no].empty()) {
        if (int_no < 32) {
            Log::printk(Log::ERROR, "RAX = %p RBX = %p RCX = %p RDX = %p\n",
                        ctx->rax, ctx->rbx, ctx->rcx, ctx->rdx);
            Log::printk(Log::ERROR, "RSI = %p RDI = %p RBP = %p RSP = %p\n",
                        ctx->rsi, ctx->rdi, ctx->rbp, ctx->rsp);
            Log::printk(Log::ERROR, "R8  = %p R9  = %p R10 = %p R11 = %p\n",
                        ctx->r8, ctx->r9, ctx->r10, ctx->r11);
            Log::printk(Log::ERROR, "R12 = %p R13 = %p R14 = %p R15 = %p\n",
                        ctx->r12, ctx->r13, ctx->r14, ctx->r15);
            Log::printk(Log::ERROR, "RIP = %p CS  = %p DS  = %p RFLAGS = %p\n",
                        ctx->rip, ctx->cs, ctx->ds, ctx->rflags);
            Kernel::panic("Unhandled exception #%d, error code 0x%X\n", int_no,
                          ctx->err_code);
        }
    } else {
        for (auto& handler : handlers[int_no]) {
            handler.handler(int_no, handler.dev_id, ctx);
        }
    }
    if (int_no >= 32) {
        IrqChip::ack(Interrupt::interrupt_to_irq(int_no));
    }
}

bool register_handler(uint32_t int_no, Interrupt::Handler& handler)
{
    if (int_no > max) {
        return false;
    }
    handlers[int_no].push_back(handler);
    return true;
}

bool unregister_handler(uint32_t int_no, const Interrupt::Handler& handler)
{
    if (int_no > max) {
        return false;
    }
    for (auto it = handlers[int_no].begin(); it != handlers[int_no].end();
         ++it) {
        auto& value = *it;
        if (&value == &handler) {
            handlers[int_no].erase(it);
            return true;
        }
    }
    return false;
}

void init()
{
    Interrupt::arch_init();
}
}  // namespace Interrupt
