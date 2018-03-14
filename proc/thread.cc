#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/process.h>
#include <proc/thread.h>

extern status_t arch_save_context(Thread* thread, struct interrupt_ctx* ctx);
extern status_t arch_load_context(Thread* thread, struct interrupt_ctx* ctx);

extern void arch_set_stack(addr_t stack);
extern addr_t arch_get_stack();

status_t save_context(Thread* thread, struct interrupt_ctx* ctx)
{
    return arch_save_context(thread, ctx);
}

status_t load_context(Thread* thread, struct interrupt_ctx* ctx)
{
    set_stack(thread->kernel_stack);
    return arch_load_context(thread, ctx);
}

Thread::Thread(Process* p)
{
    parent = p;
    parent->add_thread(this);
}

Thread::~Thread()
{
}

void Thread::load(addr_t entry)
{
    addr_t phys_stack = Memory::Physical::allocate();
    Memory::Virtual::map(0x1000, phys_stack, PAGE_WRITABLE | PAGE_USER);
    String::memset((void*)0x1000, 0, 4096);
    String::memset(&cpu_ctx, 0, sizeof(cpu_ctx));
    cpu_ctx.rip = entry;
    cpu_ctx.cs = 0x18 | 3;
    cpu_ctx.ds = 0x20 | 3;
    cpu_ctx.ss = 0x20 | 3;
    cpu_ctx.rsp = cpu_ctx.rbp = 0x2000;
    cpu_ctx.rflags = 0x200;
    kernel_stack = reinterpret_cast<addr_t>(new uint8_t[0x1000]) + 0x1000;
}

void set_stack(addr_t stack)
{
    return arch_set_stack(stack);
}

addr_t get_stack()
{
    return arch_get_stack();
}
