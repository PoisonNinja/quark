#pragma once

#include <drivers/irqchip/irqchip.h>

namespace irqchip
{
class intel8259 : public irqchip
{
public:
    bool enable() override;
    bool disable() override;
    bool mask(uint32_t irq) override;
    bool unmask(uint32_t irq) override;
    bool ack(uint32_t irq) override;
    bool spurious() override;
};
} // namespace irqchip
