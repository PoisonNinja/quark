#pragma once

#include <types.h>

namespace IrqChip
{
class IrqChip
{
public:
    virtual bool enable()             = 0;
    virtual bool disable()            = 0;
    virtual bool mask(uint32_t irq)   = 0;
    virtual bool unmask(uint32_t irq) = 0;
    virtual bool ack(uint32_t irq)    = 0;
};

// For kernel use
bool mask(uint32_t irq);
bool unmask(uint32_t irq);
bool ack(uint32_t irq);

bool set_irqchip(IrqChip& chip);

// For driver use
void spurious(uint32_t irq);
} // namespace IrqChip