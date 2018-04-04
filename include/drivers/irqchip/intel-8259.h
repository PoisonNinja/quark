#pragma once

#include <drivers/irqchip/irqchip.h>

namespace IrqChip
{
class Intel8259 : public IrqChip
{
public:
    status_t enable() override;
    status_t disable() override;
    status_t mask(uint32_t irq) override;
    status_t unmask(uint32_t irq) override;
    status_t ack(uint32_t irq) override;
    bool spurious() override;
};
}