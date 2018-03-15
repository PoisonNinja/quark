#include <arch/cpu/gdt.h>
#include <cpu/interrupt.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/thread.h>

status_t Thread::save_context(struct interrupt_ctx* ctx)
{
    if (!ctx) {
        return FAILURE;
    }
    struct thread_ctx* registers = &cpu_ctx;
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

status_t Thread::load_context(struct interrupt_ctx* ctx)
{
    set_stack(kernel_stack);
    if (!ctx) {
        return FAILURE;
    }
    struct thread_ctx* registers = &cpu_ctx;
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

void Thread::load(addr_t entry, int argc, const char* argv[], int envc,
                  const char* envp[])
{
    addr_t phys_stack = Memory::Physical::allocate();
    Memory::Virtual::map(0x1000, phys_stack, PAGE_WRITABLE | PAGE_USER);
    String::memset((void*)0x1000, 0, 4096);
    String::memset(&cpu_ctx, 0, sizeof(cpu_ctx));
    cpu_ctx.rip = entry;
    cpu_ctx.rdi = argc;
    cpu_ctx.rsi = reinterpret_cast<uint64_t>(argv);
    cpu_ctx.rdx = envc;
    cpu_ctx.rcx = reinterpret_cast<uint64_t>(envp);
    cpu_ctx.cs = 0x18 | 3;
    cpu_ctx.ds = 0x20 | 3;
    cpu_ctx.ss = 0x20 | 3;
    cpu_ctx.rsp = cpu_ctx.rbp = 0x2000;
    cpu_ctx.rflags = 0x200;
    kernel_stack = reinterpret_cast<addr_t>(new uint8_t[0x1000]) + 0x1000;
}

void arch_set_stack(addr_t stack)
{
    TSS::set_stack(stack);
}

addr_t arch_get_stack()
{
    return TSS::get_stack();
}
