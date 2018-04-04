#pragma once

#include <types.h>

namespace IrqChip
{
class IrqChip
{
public:
    virtual status_t enable() = 0;
    virtual status_t disable() = 0;
    virtual status_t mask(uint32_t irq) = 0;
    virtual status_t unmask(uint32_t irq) = 0;
    virtual status_t ack(uint32_t irq) = 0;
    virtual bool spurious() = 0;
};

status_t mask(uint32_t irq);
status_t unmask(uint32_t irq);
status_t ack(uint32_t irq);

status_t set_irqchip(IrqChip& chip);
}