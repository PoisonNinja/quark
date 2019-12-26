#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>
#include <proc/sched.h>
#include <proc/signal.h>
#include <proc/thread.h>

namespace interrupt
{
static libcxx::list<interrupt::handler, &interrupt::handler::node>
    handlers[max];

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

void dispatch(int int_no, struct interrupt_context* ctx)
{
    if (handlers[int_no].empty()) {
        if (is_exception(int_no)) {
            dump(ctx);
            if (is_userspace(ctx)) {
                scheduler::get_current_thread()->send_signal(SIGSEGV);
                return scheduler::get_current_thread()->handle_signal(ctx);
            } else {
                kernel::panic("Unhandled exception, system halted\n");
            }
        }
    } else {
        for (auto& handler : handlers[int_no]) {
            handler.func(int_no, handler.dev_id, ctx);
        }
    }
    if (!is_exception(int_no)) {
        irqchip::ack(interrupt::interrupt_to_irq(int_no));
        if (scheduler::online() && scheduler::get_current_thread()->get_flag(
                                       thread_flag::RESCHEDULE)) {
            scheduler::get_current_thread()->set_flag(thread_flag::RESCHEDULE,
                                                      false);
            scheduler::switch_next();
        }
    }
    if (scheduler::online()) {
        if (scheduler::get_current_thread()->is_signal_pending()) {
            scheduler::get_current_thread()->handle_signal(ctx);
        }
    }
}

bool register_handler(uint32_t int_no, interrupt::handler& handler)
{
    if (int_no > max) {
        return false;
    }
    handlers[int_no].push_back(handler);
    return true;
}

bool unregister_handler(uint32_t int_no, const interrupt::handler& handler)
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
    interrupt::arch_init();
}
} // namespace interrupt
