#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>

namespace IrqChip
{
static IrqChip* current_chip = nullptr;
static uint8_t interrupt_mask[INTERRUPT_MAX] = {
    0};  // TODO: Convert to bitfield

status_t mask(uint32_t irq)
{
    if (irq >= INTERRUPT_MAX) {
        return FAILURE;
    }
    if (!interrupt_mask[irq]) {
        interrupt_mask[irq] = 1;
        return current_chip->mask(irq);
    } else {
        return FAILURE;
    }
}

status_t unmask(int irq)
{
    if (irq >= INTERRUPT_MAX) {
        return FAILURE;
    }
    if (interrupt_mask[irq]) {
        interrupt_mask[irq] = 0;
        return current_chip->unmask(irq);
    } else {
        return FAILURE;
    }
}

status_t ack(int irq)
{
    if (irq >= INTERRUPT_MAX) {
        return FAILURE;
    }
    return current_chip->ack(irq);
}

status_t set_irqchip(IrqChip& chip)
{
    if (current_chip) {
        current_chip->disable();
    }
    for (int i = 0; i < INTERRUPT_MAX; i++) {
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
