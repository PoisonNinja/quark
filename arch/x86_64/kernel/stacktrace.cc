#include <kernel.h>
#include <kernel/symbol.h>

void arch_do_stack_trace()
{
    uint64_t* rbp = NULL;
    __asm__("mov %%rbp, %0" : "=r"(rbp));
    int frame = 0;
    while (rbp) {
        uint64_t rip = rbp[1];
        if (!rip)
            break;
        rbp = (uint64_t*)rbp[0];
        auto data = Symbols::resolve_addr_fuzzy(rip);
        Log::printk(Log::LogLevel::ERROR, "Frame #%d: %s+%llX\n", frame++,
                    data.first, data.second);
    }
}