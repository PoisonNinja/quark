#include <kernel.h>
#include <kernel/symbol.h>
#include <mm/virtual.h>

void arch_do_stack_trace()
{
    addr_t* bp = NULL;
    __asm__("mov %%rbp, %0" : "=r"(bp));
    int frame = 0;
    while (bp && memory::virt::test(reinterpret_cast<addr_t>(bp))) {
        // The return address is stored right below the previous base pointer.
        addr_t rip = bp[1];
        if (!rip)
            break;
        // The base pointer points to the previous base pointer
        bp                  = (addr_t*)bp[0];
        auto [name, offset] = symbols::resolve_addr_fuzzy(rip);
        if (!name) {
            break;
        }
        log::printk(log::log_level::ERROR, "Frame #%d: %s+%zX\n", frame++, name,
                    offset);
    }
}
