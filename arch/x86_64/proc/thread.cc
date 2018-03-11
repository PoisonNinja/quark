#include <arch/cpu/gdt.h>
#include <cpu/interrupt.h>
#include <proc/thread.h>

status_t arch_save_context(Thread* thread, struct interrupt_ctx* ctx)
{
    if (!thread || !ctx) {
        return FAILURE;
    }
    struct thread_ctx* registers = &thread->cpu_ctx;
    registers->rax = ctx->rax;
    registers->rbx = ctx->rbx;
    registers->rcx = ctx->rcx;
    registers->rdx = ctx->rdx;
    registers->rdi = ctx->rdi;
    registers->rsi = ctx->rsi;
    registers->rsp = ctx->rsp;
    registers->rbp = ctx->rbp;
    registers->r8 = ctx->r8;
    registers->r9 = ctx->r9;
    registers->r10 = ctx->r10;
    registers->r11 = ctx->r11;
    registers->r12 = ctx->r12;
    registers->r13 = ctx->r13;
    registers->rip = ctx->rip;
    registers->rflags = ctx->rflags;
    registers->ss = ctx->ss;
    registers->cs = ctx->cs;
    registers->ds = ctx->ds;
    return SUCCESS;
}

status_t arch_load_context(Thread* thread, struct interrupt_ctx* ctx)
{
    if (!thread || !ctx) {
        return FAILURE;
    }
    struct thread_ctx* registers = &thread->cpu_ctx;
    ctx->rax = registers->rax;
    ctx->rbx = registers->rbx;
    ctx->rcx = registers->rcx;
    ctx->rdx = registers->rdx;
    ctx->rdi = registers->rdi;
    ctx->rsi = registers->rsi;
    ctx->rsp = registers->rsp;
    ctx->rbp = registers->rbp;
    ctx->r8 = registers->r8;
    ctx->r9 = registers->r9;
    ctx->r10 = registers->r10;
    ctx->r11 = registers->r11;
    ctx->r12 = registers->r12;
    ctx->r13 = registers->r13;
    ctx->rip = registers->rip;
    ctx->rflags = registers->rflags;
    ctx->ss = registers->ss;
    ctx->cs = registers->cs;
    ctx->ds = registers->ds;
    return SUCCESS;
}

void arch_set_stack(addr_t stack)
{
    TSS::set_stack(stack);
}

addr_t arch_get_stack()
{
    return TSS::get_stack();
}
