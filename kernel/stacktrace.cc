#include <cpu/interrupt.h>
#include <kernel/stacktrace.h>

extern void arch_do_stack_trace();

void do_stack_trace()
{
    int flags;
    Interrupt::save(flags);
    Interrupt::disable();

    arch_do_stack_trace();

    Interrupt::restore(flags);
}