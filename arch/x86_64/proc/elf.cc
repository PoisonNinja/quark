#include <lib/string.h>
#include <proc/binfmt/elf.h>

namespace elf
{
void setup_registers(struct thread_context& ctx, addr_t entry, uint64_t argc,
                     addr_t argv, addr_t envp, addr_t uthread, addr_t stack)
{
    libcxx::memset(&ctx, 0, sizeof(ctx));
    ctx.rip = entry;
    ctx.rdi = argc;
    ctx.rsi = argv;
    ctx.rdx = envp;
    ctx.cs  = 0x20 | 3;
    ctx.ds  = 0x18 | 3;
    ctx.ss  = 0x18 | 3;
    ctx.fs  = uthread;
    ctx.rsp = ctx.rbp = stack;
    ctx.rflags        = 0x200;
}
} // namespace elf
