#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>

namespace IrqChip
{
static IrqChip* current_chip = nullptr;
static uint8_t interrupt_mask[Interrupt::max] = {
    0};  // TODO: Convert to bitfield

status_t mask(uint32_t irq)
{
    if (irq >= Interrupt::max) {
        return FAILURE;
    }
    if (!interrupt_mask[irq]) {
        interrupt_mask[irq] = 1;
        return current_chip->mask(irq);
    } else {
        return FAILURE;
    }
}

status_t unmask(uint32_t irq)
{
    if (irq >= Interrupt::max) {
        return FAILURE;
    }
    if (interrupt_mask[irq]) {
        interrupt_mask[irq] = 0;
        return current_chip->unmask(irq);
    } else {
        return FAILURE;
    }
}

status_t ack(uint32_t irq)
{
    if (irq >= Interrupt::max) {
        return FAILURE;
    }
    return current_chip->ack(irq);
}

status_t set_irqchip(IrqChip& chip)
{
    if (current_chip) {
        current_chip->disable();
    }
    for (uint32_t i = 0; i < Interrupt::max; i++) {
        if (interrupt_mask[i]) {
            chip.mask(i);
        } else {
            chip.unmask(i);
        }
    }
    current_chip = &chip;
    current_chip->enable();
    return SUCCESS;
}
}
