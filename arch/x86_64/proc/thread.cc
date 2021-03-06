#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <arch/mm/layout.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/process.h>
#include <proc/thread.h>
#include <proc/uthread.h>

namespace
{
extern "C" void syscall_return();

void set_thread_base(addr_t base)
{
    cpu::x86_64::wrmsr(cpu::x86_64::msr_kernel_gs_base, base);
}

unsigned prepare_stack(addr_t* stack, addr_t rip, addr_t rbp)
{
    stack[-1] = reinterpret_cast<addr_t>(rip); // RIP
    stack[-2] = 0x200;                         // RFLAGS
    stack[-3] = reinterpret_cast<addr_t>(rbp); // RBP
    stack[-4] = stack[-5] = stack[-6] = stack[-7] = stack[-8] = 0;
    return 64;
}
} // namespace

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

thread_context thread::get_context()
{
    // A copy
    return this->tcb.tcontext;
}

void thread::set_context(thread_context& context)
{
    this->tcb.tcontext = context;
}

void thread::fork_init()
{
    this->tcb.kernel_stack -= sizeof(struct interrupt_context);
    // sys_fork has already set everything up for us in tcontext
    decode_tcontext(
        reinterpret_cast<struct interrupt_context*>(this->tcb.kernel_stack),
        &this->tcb.tcontext);
    addr_t* stack = reinterpret_cast<addr_t*>(this->tcb.kernel_stack);
    // Time to hack the stack
    this->tcb.kernel_stack -=
        prepare_stack(stack, reinterpret_cast<addr_t>(syscall_return), 0);
}

addr_t thread::get_stack()
{
    return this->tcb.kernel_stack;
}

void thread::set_stack(addr_t addr)
{
    this->tcb.kernel_stack = addr;
}

void thread::save_state(interrupt_context* ctx)
{
    encode_tcontext(ctx, &this->tcb.tcontext);
}

void thread::load_state(interrupt_context* ctx)
{
    decode_tcontext(ctx, &this->tcb.tcontext);
}

extern "C" void do_task_switch(addr_t* old_stack, addr_t* new_stack);

void thread::switch_thread(thread* next)
{
    cpu::x86_64::TSS::set_stack(next->tcb.kernel_stack_base);
    set_thread_base(reinterpret_cast<addr_t>(&next->tcb));
    do_task_switch(&this->tcb.kernel_stack, &next->tcb.kernel_stack);
}

thread* create_thread(process* p, void (*entry_point)(void*), void* data)
{
    thread* kthread = p->create_thread();
    struct thread_context ctx;
    libcxx::memset(&ctx, 0, sizeof(ctx));
    addr_t* stack = reinterpret_cast<addr_t*>(kthread->get_stack());
    int offset    = prepare_stack(stack, reinterpret_cast<addr_t>(entry_point),
                               reinterpret_cast<addr_t>(stack));
    kthread->set_stack(reinterpret_cast<addr_t>(stack) - offset);
    return kthread;
}

extern "C" void load_register_state(struct interrupt_context* ctx);

void load_registers(struct thread_context& tcontext)
{
    struct interrupt_context ctx;
    decode_tcontext(&ctx, &tcontext);
    load_register_state(&ctx);
}
