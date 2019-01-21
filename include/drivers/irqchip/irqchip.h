#pragma once

#include <types.h>

namespace irqchip
{
class irqchip
{
public:
    virtual bool enable()             = 0;
    virtual bool disable()            = 0;
    virtual bool mask(uint32_t irq)   = 0;
    virtual bool unmask(uint32_t irq) = 0;
    virtual bool ack(uint32_t irq)    = 0;
    virtual bool spurious()           = 0;
};

bool mask(uint32_t irq);
bool unmask(uint32_t irq);
bool ack(uint32_t irq);

bool set_irqchip(irqchip& chip);
} // namespace irqchip
