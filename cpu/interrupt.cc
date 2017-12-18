#include <cpu/interrupt.h>
#include <stdatomic.h>

namespace Interrupt
{
static List<Interrupt::Handler, &Interrupt::Handler::node>
    handlers[INTERRUPT_MAX];

static _Atomic int interrupt_depth = 1;

extern void arch_disable(void);
extern void arch_enable(void);

int disable(void)
{
    if (!atomic_fetch_add(&interrupt_depth, 1))
        Interrupt::arch_disable();
    return interrupt_depth;
}

int enable(void)
{
    if (atomic_fetch_sub(&interrupt_depth, 1) == 1)
        Interrupt::arch_enable();
    return interrupt_depth;
}

status_t register_handler(uint32_t int_no, Interrupt::Handler& handler)
{
    if (int_no > INTERRUPT_MAX) {
        return FAILURE;
    }
    handlers[int_no].push_back(handler);
    return SUCCESS;
}

status_t unregister_handler(uint32_t int_no, const Interrupt::Handler& handler)
{
    if (int_no > INTERRUPT_MAX) {
        return FAILURE;
    }
    for (auto it = handlers[int_no].begin(); it != handlers[int_no].end();
         ++it) {
        auto& value = *it;
        if (&value == &handler) {
            handlers[int_no].erase(it);
            return SUCCESS;
        }
    }
    return FAILURE;
}
}  // namespace Interrupt
