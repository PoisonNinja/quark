#include <cpu/interrupt.h>

namespace Interrupt
{
void arch_disable(void)
{
    __asm__("cli");
}

void arch_enable(void)
{
    __asm__("sti");
}
};  // namespace Interrupt
