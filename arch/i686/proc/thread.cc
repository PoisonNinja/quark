#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <arch/mm/layout.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/elf.h>
#include <proc/process.h>
#include <proc/thread.h>

void save_context(struct InterruptContext* ctx,
                  struct ThreadContext* thread_ctx)
{
}

void load_context(struct InterruptContext* ctx,
                  struct ThreadContext* thread_ctx)
{
}

bool Thread::load(addr_t binary, int argc, const char* argv[], int envc,
                  const char* envp[], struct ThreadContext& ctx)
{
    return false;
}

void set_stack(addr_t stack)
{
}

addr_t get_stack()
{
    return 0;
}

void set_thread_base(Thread* thread)
{
}

Thread* create_kernel_thread(Process* p, void (*entry_point)(void*), void* data)
{
    return nullptr;
}

extern "C" void load_register_state(struct InterruptContext* ctx);

void load_registers(struct ThreadContext& cpu_ctx)
{
}
