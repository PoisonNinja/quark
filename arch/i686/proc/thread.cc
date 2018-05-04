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
    thread_ctx->edi = ctx->edi;
    thread_ctx->esi = ctx->esi;
    thread_ctx->ebp = ctx->ebp;
    thread_ctx->ebx = ctx->ebx;
    thread_ctx->edx = ctx->edx;
    thread_ctx->ecx = ctx->ecx;
    thread_ctx->eax = ctx->eax;
    thread_ctx->eip = ctx->eip;
    thread_ctx->eflags = ctx->eflags;
    thread_ctx->ds = ctx->ds;
    thread_ctx->cs = ctx->cs;
    thread_ctx->esp = ctx->esp;
    thread_ctx->ss = ctx->ss;
}

void load_context(struct InterruptContext* ctx,
                  struct ThreadContext* thread_ctx)
{
    ctx->edi = thread_ctx->edi;
    ctx->esi = thread_ctx->esi;
    ctx->ebp = thread_ctx->ebp;
    ctx->ebx = thread_ctx->ebx;
    ctx->edx = thread_ctx->edx;
    ctx->ecx = thread_ctx->ecx;
    ctx->eax = thread_ctx->eax;
    ctx->eip = thread_ctx->eip;
    ctx->eflags = thread_ctx->eflags;
    ctx->ds = thread_ctx->ds;
    ctx->cs = thread_ctx->cs;
    ctx->esp = thread_ctx->esp;
    ctx->ss = thread_ctx->ss;
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
    Thread* thread = new Thread(p);
    String::memset(&thread->cpu_ctx, 0, sizeof(thread->cpu_ctx));
    addr_t* stack = reinterpret_cast<addr_t*>(new uint8_t[0x1000] + 0x1000);
    thread->cpu_ctx.eip = reinterpret_cast<addr_t>(entry_point);
    thread->cpu_ctx.ebp = reinterpret_cast<addr_t>(stack);
    thread->cpu_ctx.esp = reinterpret_cast<addr_t>(stack - 1);
    thread->cpu_ctx.cs = 0x8;
    thread->cpu_ctx.ds = 0x10;
    thread->cpu_ctx.eflags = 0x200;
    thread->kernel_stack =
        reinterpret_cast<addr_t>(new uint8_t[0x1000]) + 0x1000;
    return thread;
}

extern "C" void load_register_state(struct InterruptContext* ctx);

void load_registers(struct ThreadContext& cpu_ctx)
{
}
