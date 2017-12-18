#pragma once

#include <lib/list.h>
#include <types.h>

namespace Interrupt
{
struct interrupt_ctx;

#define INTERRUPT_MAX 256U

typedef void (*interrupt_handler_t)(int, void *, struct interrupt_ctx *);

struct Handler {
    const interrupt_handler_t handler;
    const char *dev_name;
    const void *dev_id;
    Node<Handler> node;
};

status_t register_interrupt_handler(uint32_t int_no,
                                    Interrupt::Handler &handler);
};  // namespace Interrupt
