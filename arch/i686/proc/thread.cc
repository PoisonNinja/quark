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
    addr_t entry = ELF::load(binary);
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
    Memory::Virtual::map_range(argv_zone, argv_size,
                               PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    Memory::Virtual::map_range(envp_zone, envp_size,
                               PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    Memory::Virtual::map_range(stack_zone, 0x1000,
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
    String::memset(&ctx, 0, sizeof(ctx));
    ctx.eip = entry;
    ctx.cs = 0x20 | 3;
    ctx.ds = 0x18 | 3;
    ctx.ss = 0x18 | 3;
    ctx.esp = ctx.ebp = reinterpret_cast<addr_t>(stack_zone) + 0x1000;
    // Load in arguments
    uint32_t* stack = reinterpret_cast<uint32_t*>(ctx.esp);
    stack[-2] = argc;
    stack[-3] = reinterpret_cast<uint64_t>(target_argv);
    stack[-4] = envc;
    stack[-5] = reinterpret_cast<uint64_t>(target_envp);
    ctx.esp -= 20;
    ctx.eflags = 0x200;
    return true;
}

void set_stack(addr_t stack)
{
    CPU::X86::TSS::set_stack(stack);
}

addr_t get_stack()
{
    return CPU::X86::TSS::get_stack();
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
    struct InterruptContext ctx;
    load_context(&ctx, &cpu_ctx);
    load_register_state(&ctx);
}
