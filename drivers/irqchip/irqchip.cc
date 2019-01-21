#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>

namespace irqchip
{
static irqchip* current_chip                  = nullptr;
static uint8_t interrupt_mask[interrupt::max] = {
    0}; // TODO: Convert to bitfield

bool mask(uint32_t irq)
{
    if (irq >= interrupt::max) {
        return false;
    }
    if (!interrupt_mask[irq]) {
        interrupt_mask[irq] = 1;
        return current_chip->mask(irq);
    } else {
        return false;
    }
}

bool unmask(uint32_t irq)
{
    if (irq >= interrupt::max) {
        return false;
    }
    if (interrupt_mask[irq]) {
        interrupt_mask[irq] = 0;
        return current_chip->unmask(irq);
    } else {
        return false;
    }
}

bool ack(uint32_t irq)
{
    if (irq >= interrupt::max) {
        return false;
    }
    return current_chip->ack(irq);
}

bool set_irqchip(irqchip& chip)
{
    if (current_chip) {
        current_chip->disable();
    }
    for (uint32_t i = 0; i < interrupt::max; i++) {
        if (interrupt_mask[i]) {
            chip.mask(i);
        } else {
            chip.unmask(i);
        }
    }
    current_chip = &chip;
    current_chip->enable();
    return true;
}
} // namespace irqchip
