#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/string.h>
#include <proc/process.h>
#include <proc/thread.h>

void save_context(struct InterruptContext* ctx,
                  struct ThreadContext* thread_ctx)
{
    thread_ctx->rax = ctx->rax;
    thread_ctx->rbx = ctx->rbx;
    thread_ctx->rcx = ctx->rcx;
    thread_ctx->rdx = ctx->rdx;
    thread_ctx->rdi = ctx->rdi;
    thread_ctx->rsi = ctx->rsi;
    thread_ctx->rsp = ctx->rsp;
    thread_ctx->rbp = ctx->rbp;
    thread_ctx->r8 = ctx->r8;
    thread_ctx->r9 = ctx->r9;
    thread_ctx->r10 = ctx->r10;
    thread_ctx->r11 = ctx->r11;
    thread_ctx->r12 = ctx->r12;
    thread_ctx->r13 = ctx->r13;
    thread_ctx->r14 = ctx->r14;
    thread_ctx->r15 = ctx->r15;
    thread_ctx->rip = ctx->rip;
    thread_ctx->rflags = ctx->rflags;
    thread_ctx->ss = ctx->ss;
    thread_ctx->cs = ctx->cs;
    thread_ctx->ds = ctx->ds;
}

void load_context(struct InterruptContext* ctx,
                  struct ThreadContext* thread_ctx)
{
    ctx->rax = thread_ctx->rax;
    ctx->rbx = thread_ctx->rbx;
    ctx->rcx = thread_ctx->rcx;
    ctx->rdx = thread_ctx->rdx;
    ctx->rdi = thread_ctx->rdi;
    ctx->rsi = thread_ctx->rsi;
    ctx->rsp = thread_ctx->rsp;
    ctx->rbp = thread_ctx->rbp;
    ctx->r8 = thread_ctx->r8;
    ctx->r9 = thread_ctx->r9;
    ctx->r10 = thread_ctx->r10;
    ctx->r11 = thread_ctx->r11;
    ctx->r12 = thread_ctx->r12;
    ctx->r13 = thread_ctx->r13;
    ctx->r14 = thread_ctx->r14;
    ctx->r15 = thread_ctx->r15;
    ctx->rip = thread_ctx->rip;
    ctx->rflags = thread_ctx->rflags;
    ctx->ss = thread_ctx->ss;
    ctx->cs = thread_ctx->cs;
    ctx->ds = thread_ctx->ds;
}

void set_stack(addr_t stack)
{
    CPU::X64::TSS::set_stack(stack);
}

addr_t get_stack()
{
    return CPU::X64::TSS::get_stack();
}

void set_thread_base(Thread* thread)
{
    CPU::X64::wrmsr(CPU::X64::msr_kernel_gs_base,
                    reinterpret_cast<uint64_t>(thread));
}

Thread* create_kernel_thread(Process* p, void (*entry_point)(void*), void* data)
{
    Thread* thread = new Thread(p);
    String::memset(&thread->cpu_ctx, 0, sizeof(thread->cpu_ctx));
    uint64_t* stack = reinterpret_cast<uint64_t*>(new uint8_t[0x1000] + 0x1000);
    thread->cpu_ctx.rdi = reinterpret_cast<uint64_t>(data);
    thread->cpu_ctx.rip = reinterpret_cast<uint64_t>(entry_point);
    thread->cpu_ctx.rbp = reinterpret_cast<uint64_t>(stack);
    thread->cpu_ctx.rsp = reinterpret_cast<uint64_t>(stack - 1);
    thread->cpu_ctx.cs = 0x8;
    thread->cpu_ctx.ds = 0x10;
    thread->cpu_ctx.rflags = 0x200;
    thread->kernel_stack =
        reinterpret_cast<addr_t>(new uint8_t[0x1000]) + 0x1000;
    return thread;
}

extern "C" void load_register_state(struct InterruptContext* ctx);

void load_registers(struct ThreadContext& cpu_ctx)
{
    struct InterruptContext ctx;
    load_context(&ctx, &cpu_ctx);
    load_register_state(&ctx);
}
