#include <cpu/interrupt.h>

namespace Interrupt
{
static List<Interrupt::Handler, &Interrupt::Handler::node>
    handlers[INTERRUPT_MAX];

status_t register_interrupt_handler(uint32_t int_no,
                                    Interrupt::Handler& handler)
{
    if (int_no > INTERRUPT_MAX) {
        return FAILURE;
    }
    handlers[int_no].push_back(handler);
    return SUCCESS;
}

status_t unregister_interrupt_handler(uint32_t int_no,
                                      const Interrupt::Handler& handler)
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
};  // namespace Interrupt
