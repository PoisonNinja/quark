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

void set_stack(addr_t stack)
{
    return arch_set_stack(stack);
}

addr_t get_stack()
{
    return arch_get_stack();
}
