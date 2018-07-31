#include <kernel.h>
#include <kernel/init.h>

extern addr_t __initcall1_start[];
extern addr_t __initcall2_start[];
extern addr_t __initcall3_start[];
extern addr_t __initcall4_start[];
extern addr_t __initcall5_start[];
extern addr_t __initcall6_start[];
extern addr_t __initcall7_start[];
extern addr_t __initcall_end[];

static addr_t *initcall_levels[] = {
    __initcall1_start, __initcall2_start, __initcall3_start, __initcall4_start,
    __initcall5_start, __initcall6_start, __initcall7_start, __initcall_end,
};

void do_initcall(InitLevel level)
{
    Log::printk(Log::LogLevel::INFO, "Calling initcalls for level %d\n", level);
    for (addr_t *i = initcall_levels[static_cast<int>(level)];
         i < initcall_levels[static_cast<int>(level) + 1]; i++) {
        initcall_t fn = *(initcall_t *)i;
        fn();
    }
}
