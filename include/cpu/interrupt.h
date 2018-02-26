#pragma once

#include <arch/cpu/registers.h>
#include <lib/list.h>
#include <types.h>

namespace Interrupt
{
#define INTERRUPT_MAX 256  // TODO: Move this to architecture folder

inline int interrupt_to_irq(int interrupt_no)
{
    return interrupt_no - 32;  // TODO: Make this architecture independant
}

inline int irq_to_interrupt(int irq)
{
    return irq + 32;  // TODO: Make this architecture independant
}

typedef void (*interrupt_handler_t)(int, void *, struct interrupt_ctx *);

struct Handler {
    Handler(interrupt_handler_t handler, const char *dev_name, void *dev_id)
        : handler(handler), dev_name(dev_name), dev_id(dev_id){};
    interrupt_handler_t handler;
    const char *dev_name;
    void *dev_id;
    Node<Handler> node;
};

int disable();
int enable();

void dispatch(int int_no, struct interrupt_ctx *ctx);

status_t register_handler(uint32_t int_no, Interrupt::Handler &handler);
status_t unregister_handler(uint32_t int_no, const Interrupt::Handler &handler);

void init();
}  // namespace Interrupt
