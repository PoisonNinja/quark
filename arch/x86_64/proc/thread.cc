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
#include <proc/uthread.h>

void set_thread_base(thread_context* thread)
{
    cpu::x86_64::wrmsr(cpu::x86_64::msr_kernel_gs_base,
                       reinterpret_cast<uint64_t>(thread));
}

void encode_tcontext(struct interrupt_context* ctx,
                     struct thread_context* thread_ctx)
{
    thread_ctx->rax    = ctx->rax;
    thread_ctx->rbx    = ctx->rbx;
    thread_ctx->rcx    = ctx->rcx;
    thread_ctx->rdx    = ctx->rdx;
    thread_ctx->rdi    = ctx->rdi;
    thread_ctx->rsi    = ctx->rsi;
    thread_ctx->rsp    = ctx->rsp;
    thread_ctx->rbp    = ctx->rbp;
    thread_ctx->r8     = ctx->r8;
    thread_ctx->r9     = ctx->r9;
    thread_ctx->r10    = ctx->r10;
    thread_ctx->r11    = ctx->r11;
    thread_ctx->r12    = ctx->r12;
    thread_ctx->r13    = ctx->r13;
    thread_ctx->r14    = ctx->r14;
    thread_ctx->r15    = ctx->r15;
    thread_ctx->rip    = ctx->rip;
    thread_ctx->rflags = ctx->rflags;
    thread_ctx->ss     = ctx->ss;
    thread_ctx->cs     = ctx->cs;
    thread_ctx->ds     = ctx->ds;
    thread_ctx->fs     = ctx->fs;
    thread_ctx->gs     = ctx->gs;
}

void decode_tcontext(struct interrupt_context* ctx,
                     struct thread_context* thread_ctx)
{
    ctx->rax    = thread_ctx->rax;
    ctx->rbx    = thread_ctx->rbx;
    ctx->rcx    = thread_ctx->rcx;
    ctx->rdx    = thread_ctx->rdx;
    ctx->rdi    = thread_ctx->rdi;
    ctx->rsi    = thread_ctx->rsi;
    ctx->rsp    = thread_ctx->rsp;
    ctx->rbp    = thread_ctx->rbp;
    ctx->r8     = thread_ctx->r8;
    ctx->r9     = thread_ctx->r9;
    ctx->r10    = thread_ctx->r10;
    ctx->r11    = thread_ctx->r11;
    ctx->r12    = thread_ctx->r12;
    ctx->r13    = thread_ctx->r13;
    ctx->r14    = thread_ctx->r14;
    ctx->r15    = thread_ctx->r15;
    ctx->rip    = thread_ctx->rip;
    ctx->rflags = thread_ctx->rflags;
    ctx->ss     = thread_ctx->ss;
    ctx->cs     = thread_ctx->cs;
    ctx->ds     = thread_ctx->ds;
    ctx->fs     = thread_ctx->fs;
    ctx->gs     = thread_ctx->gs;
}

void thread::save_state(interrupt_context* ctx)
{
    encode_tcontext(ctx, &this->tcontext);
}

void thread::load_state(interrupt_context* ctx)
{
    decode_tcontext(ctx, &this->tcontext);
    set_stack(this->kernel_stack);
    set_thread_base(&this->tcontext);
}

void set_stack(addr_t stack)
{
    cpu::x86_64::TSS::set_stack(stack);
}

addr_t get_stack()
{
    return cpu::x86_64::TSS::get_stack();
}

thread* create_kernel_thread(process* p, void (*entry_point)(void*), void* data)
{
    thread* kthread = p->create_thread();
    libcxx::memset(&kthread->tcontext, 0, sizeof(kthread->tcontext));
    addr_t stack =
        reinterpret_cast<addr_t>(new uint8_t[0xF000] + 0xF000) & ~15UL;
    kthread->tcontext.rdi    = reinterpret_cast<addr_t>(data);
    kthread->tcontext.rip    = reinterpret_cast<addr_t>(entry_point);
    kthread->tcontext.rbp    = reinterpret_cast<addr_t>(stack);
    kthread->tcontext.rsp    = reinterpret_cast<addr_t>(stack);
    kthread->tcontext.cs     = 0x8;
    kthread->tcontext.ds     = 0x10;
    kthread->tcontext.ss     = 0x10;
    kthread->tcontext.rflags = 0x200;
    return kthread;
}

extern "C" void load_register_state(struct interrupt_context* ctx);

void load_registers(struct thread_context& tcontext)
{
    struct interrupt_context ctx;
    decode_tcontext(&ctx, &tcontext);
    load_register_state(&ctx);
}
