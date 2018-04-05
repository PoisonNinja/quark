#include <arch/cpu/gdt.h>
#include <arch/mm/layout.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/process.h>
#include <proc/thread.h>

status_t Thread::save_context(struct InterruptContext* ctx)
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
    registers->r14 = ctx->r14;
    registers->r15 = ctx->r15;
    registers->rip = ctx->rip;
    registers->rflags = ctx->rflags;
    registers->ss = ctx->ss;
    registers->cs = ctx->cs;
    registers->ds = ctx->ds;
    return SUCCESS;
}

status_t Thread::load_context(struct InterruptContext* ctx)
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
    ctx->r14 = registers->r14;
    ctx->r15 = registers->r15;
    ctx->rip = registers->rip;
    ctx->rflags = registers->rflags;
    ctx->ss = registers->ss;
    ctx->cs = registers->cs;
    ctx->ds = registers->ds;
    return SUCCESS;
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
