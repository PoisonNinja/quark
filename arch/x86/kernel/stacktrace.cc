#include <kernel.h>
#include <kernel/symbol.h>

void arch_do_stack_trace()
{
    addr_t* bp = NULL;
#ifdef X86_64
    __asm__("mov %%rbp, %0" : "=r"(bp));
#else
    __asm__("mov %%ebp, %0" : "=r"(bp));
#endif
    int frame = 0;
    while (bp) {
        addr_t rip = bp[1];
        if (!rip)
            break;
        bp = (addr_t*)bp[0];
        if (!bp)
            break;
        auto data = Symbols::resolve_addr_fuzzy(rip);
        if (!data.first) {
            break;
        }
        Log::printk(Log::LogLevel::ERROR, "Frame #%d: %s+%llX\n", frame++,
                    data.first, data.second);
    }
}
