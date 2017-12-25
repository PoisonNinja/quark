#pragma once

#include <types.h>

namespace IrqChip
{
class IrqChip
{
public:
    virtual status_t enable() = 0;
    virtual status_t disable() = 0;
    virtual status_t mask(int irq) = 0;
    virtual status_t unmask(int irq) = 0;
    virtual status_t ack(int irq) = 0;
    virtual bool spurious() = 0;
};

status_t mask(int irq);
status_t unmask(int irq);
status_t ack(int irq);

status_t set_irqchip(IrqChip& chip);
}