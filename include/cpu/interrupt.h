#pragma once

#include <lib/list.h>
#include <types.h>

struct interrupt_ctx;

namespace Interrupt
{
#define INTERRUPT_MAX 256U

typedef void (*interrupt_handler_t)(int, void *, struct interrupt_ctx *);

struct Handler {
    interrupt_handler_t handler;
    char *dev_name;
    void *dev_id;
    Node<Handler> node;
};

int disable();
int enable();

void dispatch(int int_no, struct interrupt_ctx *ctx);

status_t register_handler(uint32_t int_no, Interrupt::Handler &handler);

status_t unregister_handler(uint32_t int_no, const Interrupt::Handler &handler);
}  // namespace Interrupt
