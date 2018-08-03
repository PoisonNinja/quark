#include <kernel.h>
#include <kernel/symbol.h>

void arch_do_stack_trace()
{
    uint32_t* ebp = NULL;
    __asm__("mov %%ebp, %0" : "=r"(ebp));
    int frame = 0;
    while (ebp) {
        uint32_t eip = ebp[1];
        if (!ebp)
            break;
        ebp = (uint32_t*)ebp[0];
        if (!ebp)
            break;
        auto data = Symbols::resolve_addr_fuzzy(eip);
        if (!data.first) {
            break;
        }
        Log::printk(Log::LogLevel::ERROR, "Frame #%d: %s+%llX\n", frame++,
                    data.first, data.second);
    }
}