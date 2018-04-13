#pragma once

#include <drivers/irqchip/irqchip.h>

namespace IrqChip
{
class Intel8259 : public IrqChip
{
public:
    bool enable() override;
    bool disable() override;
    bool mask(uint32_t irq) override;
    bool unmask(uint32_t irq) override;
    bool ack(uint32_t irq) override;
    bool spurious() override;
};
}