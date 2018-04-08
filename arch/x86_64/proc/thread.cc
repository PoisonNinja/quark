#include <arch/cpu/gdt.h>
#include <arch/mm/layout.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
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

bool Thread::load(addr_t entry, int argc, const char* argv[], int envc,
                  const char* envp[])
{
    size_t argv_size = sizeof(char*) * (argc + 1);  // argv is null terminated
    size_t envp_size = sizeof(char*) * (envc + 1);  // envp is null terminated
    for (int i = 0; i < argc; i++) {
        argv_size += String::strlen(argv[i]) + 1;
    };
    for (int i = 0; i < envc; i++) {
        envp_size += String::strlen(envp[i]) + 1;
    }
    addr_t argv_zone;
    addr_t envp_zone;
    addr_t stack_zone;
    if (parent->sections->locate_range(argv_zone, USER_START, argv_size)) {
        Log::printk(Log::DEBUG, "argv located at %p\n", argv_zone);
        parent->sections->add_section(argv_zone, argv_size);
    } else {
        Log::printk(Log::ERROR, "Failed to locate argv\n");
        return false;
    }
    if (parent->sections->locate_range(envp_zone, USER_START, envp_size)) {
        Log::printk(Log::DEBUG, "envp located at %p\n", envp_zone);
        parent->sections->add_section(envp_zone, envp_size);
    } else {
        Log::printk(Log::ERROR, "Failed to locate envp\n");
        return false;
    }
    if (parent->sections->locate_range(stack_zone, USER_START, 0x1000)) {
        Log::printk(Log::DEBUG, "Stack located at %p\n", stack_zone);
        parent->sections->add_section(stack_zone, 0x1000);
    } else {
        Log::printk(Log::ERROR, "Failed to locate stack\n");
        return false;
    }
    Memory::Virtual::map(argv_zone, Memory::Physical::allocate(),
                         PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    Memory::Virtual::map(envp_zone, Memory::Physical::allocate(),
                         PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    Memory::Virtual::map(stack_zone, Memory::Physical::allocate(),
                         PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    char* target =
        reinterpret_cast<char*>(argv_zone + (sizeof(char*) * (argc + 1)));
    char** target_argv = reinterpret_cast<char**>(argv_zone);
    for (int i = 0; i < argc; i++) {
        String::strcpy(target, argv[i]);
        target_argv[i] = target;
        target += String::strlen(argv[i]) + 1;
    }
    target_argv[argc] = 0;
    target = reinterpret_cast<char*>(envp_zone + (sizeof(char*) * (envc + 1)));
    char** target_envp = reinterpret_cast<char**>(envp_zone);
    for (int i = 0; i < envc; i++) {
        String::strcpy(target, envp[i]);
        target_envp[i] = target;
        target += String::strlen(envp[i]) + 1;
    }
    target_envp[envc] = 0;
    String::memset((void*)stack_zone, 0, 0x1000);
    String::memset(&cpu_ctx, 0, sizeof(cpu_ctx));
    cpu_ctx.rip = entry;
    cpu_ctx.rdi = argc;
    cpu_ctx.rsi = reinterpret_cast<uint64_t>(target_argv);
    cpu_ctx.rdx = envc;
    cpu_ctx.rcx = reinterpret_cast<uint64_t>(target_envp);
    cpu_ctx.cs = 0x18 | 3;
    cpu_ctx.ds = 0x20 | 3;
    cpu_ctx.ss = 0x20 | 3;
    cpu_ctx.rsp = cpu_ctx.rbp = reinterpret_cast<uint64_t>(stack_zone) + 0x1000;
    cpu_ctx.rflags = 0x200;
    kernel_stack = reinterpret_cast<addr_t>(new uint8_t[0x1000]) + 0x1000;
    return true;
}

void arch_set_stack(addr_t stack)
{
    TSS::set_stack(stack);
}

addr_t arch_get_stack()
{
    return TSS::get_stack();
}
