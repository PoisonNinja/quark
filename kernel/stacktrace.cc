#include <cpu/interrupt.h>
#include <kernel/stacktrace.h>

extern void arch_do_stack_trace();

void do_stack_trace()
{
    int flags;
    interrupt::save(flags);
    interrupt::disable();

    arch_do_stack_trace();

    interrupt::restore(flags);
}