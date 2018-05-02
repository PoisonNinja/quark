#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>
#include <atomic>

namespace Interrupt
{
static List<Interrupt::Handler, &Interrupt::Handler::node> handlers[max];

static std::atomic<int> interrupt_depth(1);

extern void arch_init();

void save(int& store)
{
    store = interrupts_enabled();
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
        if (is_exception(int_no)) {
            dump(ctx);
            Kernel::panic("Unhandled exception, system halted\n");
        }
    } else {
        for (auto& handler : handlers[int_no]) {
            handler.handler(int_no, handler.dev_id, ctx);
        }
    }
    if (!is_exception(int_no)) {
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
