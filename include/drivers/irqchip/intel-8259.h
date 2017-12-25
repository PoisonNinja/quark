#pragma once

#include <drivers/irqchip/irqchip.h>

namespace IrqChip
{
class Intel8259 : public IrqChip
{
public:
    status_t enable() override;
    status_t disable() override;
    status_t mask(int irq) override;
    status_t unmask(int irq) override;
    status_t ack(int irq) override;
    bool spurious() override;
};
}