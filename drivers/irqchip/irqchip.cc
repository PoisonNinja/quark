#include <cpu/interrupt.h>
#include <drivers/irqchip/irqchip.h>
#include <kernel.h>

namespace IrqChip
{
static IrqChip* current_chip               = nullptr;
static bool interrupt_mask[Interrupt::max] = {
    false}; // TODO: Convert to bitfield

bool mask(uint32_t irq)
{
    if (irq >= Interrupt::max) {
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
    if (irq >= Interrupt::max) {
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
    if (irq >= Interrupt::max) {
        return false;
    }
    return current_chip->ack(irq);
}

void spurious(uint32_t irq)
{
    Log::printk(Log::LogLevel::WARNING,
                "irqchip: Spurious interrupt received on line %u\n", irq);
    // TODO: Track them
}

bool set_irqchip(IrqChip& chip)
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
    return true;
}
} // namespace IrqChip
