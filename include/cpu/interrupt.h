#pragma once

#include <arch/cpu/registers.h>
#include <lib/functional.h>
#include <lib/list.h>
#include <types.h>

namespace Interrupt
{
constexpr size_t max = 256; // TODO: Move this to architecture folder

inline int interrupt_to_irq(int interrupt_no)
{
    return interrupt_no - 32; // TODO: Make this architecture independant
}

inline int irq_to_interrupt(int irq)
{
    return irq + 32; // TODO: Make this architecture independant
}

using interrupt_handler_t = void (*)(int, void *, struct InterruptContext *);

struct Handler {
    Handler(interrupt_handler_t handler, const char *dev_name, void *dev_id)
        : handler(handler)
        , dev_name(dev_name)
        , dev_id(dev_id){};
    interrupt_handler_t handler;
    libcxx::function<void(int, void *, struct InterruptContext *)> handler_v2;
    const char *dev_name;
    void *dev_id;
    libcxx::node<Handler> node;
};

void disable();
void enable();

void save(int &store);
void restore(int &store);

void dispatch(int int_no, struct InterruptContext *ctx);

void dump(struct InterruptContext *ctx);

bool interrupts_enabled();
bool is_exception(int int_no);

bool is_userspace(struct InterruptContext *ctx);

bool register_handler(uint32_t int_no, Interrupt::Handler &handler);
bool unregister_handler(uint32_t int_no, const Interrupt::Handler &handler);

void init();
} // namespace Interrupt
