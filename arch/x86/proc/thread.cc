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

extern "C" void signal_return();
void* signal_return_location = (void*)&signal_return;

#define ROUND_UP(x, y) ((((x) + ((y)-1)) / y) * y)

#ifdef X86_64
void set_thread_base(ThreadContext* thread)
{
    CPU::X86::wrmsr(CPU::X86::msr_kernel_gs_base,
                    reinterpret_cast<uint64_t>(thread));
}
#endif

void encode_tcontext(struct InterruptContext* ctx,
                     struct ThreadContext* thread_ctx)
{
#ifdef X86_64
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
#else
    thread_ctx->edi    = ctx->edi;
    thread_ctx->esi    = ctx->esi;
    thread_ctx->ebp    = ctx->ebp;
    thread_ctx->ebx    = ctx->ebx;
    thread_ctx->edx    = ctx->edx;
    thread_ctx->ecx    = ctx->ecx;
    thread_ctx->eax    = ctx->eax;
    thread_ctx->eip    = ctx->eip;
    thread_ctx->eflags = ctx->eflags;
    thread_ctx->ds     = ctx->ds;
    thread_ctx->cs     = ctx->cs;
    thread_ctx->esp    = ctx->esp;
    thread_ctx->ss     = ctx->ss;
#endif
}

void decode_tcontext(struct InterruptContext* ctx,
                     struct ThreadContext* thread_ctx)
{
#ifdef X86_64
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
#else
    ctx->edi           = thread_ctx->edi;
    ctx->esi           = thread_ctx->esi;
    ctx->ebp           = thread_ctx->ebp;
    ctx->ebx           = thread_ctx->ebx;
    ctx->edx           = thread_ctx->edx;
    ctx->ecx           = thread_ctx->ecx;
    ctx->eax           = thread_ctx->eax;
    ctx->eip           = thread_ctx->eip;
    ctx->eflags        = thread_ctx->eflags;
    ctx->ds            = thread_ctx->ds;
    ctx->cs            = thread_ctx->cs;
    ctx->esp           = thread_ctx->esp;
    ctx->ss            = thread_ctx->ss;
#endif
}

void save_context(InterruptContext* ctx, struct ThreadContext* tcontext)
{
    encode_tcontext(ctx, tcontext);
#ifndef X86_64
    tcontext->fs = CPU::X86::GDT::get_fs();
    tcontext->gs = CPU::X86::GDT::get_gs();
#endif
}

void load_context(InterruptContext* ctx, struct ThreadContext* tcontext)
{
    decode_tcontext(ctx, tcontext);
    set_stack(tcontext->kernel_stack);
#ifdef X86_64
    set_thread_base(tcontext);
#else
    CPU::X86::GDT::set_fs(tcontext->fs);
    CPU::X86::GDT::set_gs(tcontext->gs);
#endif
}

bool Thread::load(addr_t binary, int argc, const char* argv[], int envc,
                  const char* envp[], struct ThreadContext& ctx)
{
    auto [success, entry] = ELF::load(binary);

    /*
     * ELF::load returns a pair. The first parameter (bool) indicates status,
     * and second parameter is the entry address. If the first parameter is
     * false, the second parameter is undefined (but usually 0)
     */
    if (!success) {
        return false;
    }

    size_t argv_size = sizeof(char*) * (argc + 1); // argv is null terminated
    size_t envp_size = sizeof(char*) * (envc + 1); // envp is null terminated
    for (int i = 0; i < argc; i++) {
        argv_size += libcxx::strlen(argv[i]) + 1;
    };
    for (int i = 0; i < envc; i++) {
        envp_size += libcxx::strlen(envp[i]) + 1;
    }
    addr_t argv_zone;
    addr_t envp_zone;
    addr_t stack_zone;
    addr_t sigreturn_zone;
    addr_t tls_zone;

    size_t tls_raw_size = ROUND_UP(parent->tls_memsz, parent->tls_alignment);
    size_t tls_size     = tls_raw_size + sizeof(struct uthread);
    tls_size            = ROUND_UP(tls_size, parent->tls_alignment);

    if (parent->sections->locate_range(argv_zone, USER_START, argv_size)) {
        Log::printk(Log::LogLevel::DEBUG, "argv located at %p\n", argv_zone);
        parent->sections->add_section(argv_zone, argv_size);
    } else {
        Log::printk(Log::LogLevel::ERROR, "Failed to locate argv\n");
        return false;
    }
    if (parent->sections->locate_range(envp_zone, USER_START, envp_size)) {
        Log::printk(Log::LogLevel::DEBUG, "envp located at %p\n", envp_zone);
        parent->sections->add_section(envp_zone, envp_size);
    } else {
        Log::printk(Log::LogLevel::ERROR, "Failed to locate envp\n");
        return false;
    }
    if (parent->sections->locate_range(stack_zone, USER_START, 0x1000)) {
        Log::printk(Log::LogLevel::DEBUG, "Stack located at %p\n", stack_zone);
        parent->sections->add_section(stack_zone, 0x1000);
    } else {
        Log::printk(Log::LogLevel::ERROR, "Failed to locate stack\n");
        return false;
    }
    if (parent->sections->locate_range(sigreturn_zone, USER_START, 0x1000)) {
        Log::printk(Log::LogLevel::DEBUG, "Sigreturn page located at %p\n",
                    sigreturn_zone);
        parent->sections->add_section(sigreturn_zone, 0x1000);
    } else {
        Log::printk(Log::LogLevel::ERROR, "Failed to locate sigreturn page\n");
        return false;
    }
    if (parent->sections->locate_range(tls_zone, USER_START, tls_size)) {
        Log::printk(Log::LogLevel::DEBUG, "TLS copy located at %p\n", tls_zone);
        parent->sections->add_section(tls_zone, tls_size);
    } else {
        Log::printk(Log::LogLevel::ERROR, "Failed to locate TLS copy\n");
        return false;
    }
    Memory::Virtual::map_range(argv_zone, argv_size,
                               PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    Memory::Virtual::map_range(envp_zone, envp_size,
                               PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    Memory::Virtual::map_range(stack_zone, 0x1000,
                               PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    Memory::Virtual::map_range(sigreturn_zone, 0x1000,
                               PAGE_USER | PAGE_WRITABLE);
    Memory::Virtual::map_range(tls_zone, tls_size,
                               PAGE_USER | PAGE_NX | PAGE_WRITABLE);

    this->parent->sigreturn = sigreturn_zone;

    // Copy in sigreturn trampoline code
    libcxx::memcpy(reinterpret_cast<void*>(sigreturn_zone),
                   signal_return_location, 0x1000);

    // Make it unwritable
    Memory::Virtual::protect(sigreturn_zone, PAGE_USER);

    // Copy TLS data into thread specific data
    libcxx::memcpy((void*)tls_zone, (void*)parent->tls_base,
                   parent->tls_filesz);
    libcxx::memset((void*)(tls_zone + parent->tls_filesz), 0,
                   parent->tls_memsz - parent->tls_filesz);

    struct uthread* uthread =
        reinterpret_cast<struct uthread*>(tls_zone + tls_raw_size);
    uthread->self = uthread;

    char* target =
        reinterpret_cast<char*>(argv_zone + (sizeof(char*) * (argc + 1)));
    char** target_argv = reinterpret_cast<char**>(argv_zone);
    for (int i = 0; i < argc; i++) {
        libcxx::strcpy(target, argv[i]);
        target_argv[i] = target;
        target += libcxx::strlen(argv[i]) + 1;
    }
    target_argv[argc] = 0;
    target = reinterpret_cast<char*>(envp_zone + (sizeof(char*) * (envc + 1)));
    char** target_envp = reinterpret_cast<char**>(envp_zone);
    for (int i = 0; i < envc; i++) {
        libcxx::strcpy(target, envp[i]);
        target_envp[i] = target;
        target += libcxx::strlen(envp[i]) + 1;
    }
    target_envp[envc] = 0;
    libcxx::memset((void*)stack_zone, 0, 0x1000);

    libcxx::memset(&ctx, 0, sizeof(ctx));
#ifdef X86_64
    ctx.rip = entry;
    ctx.rdi = argc;
    ctx.rsi = reinterpret_cast<uint64_t>(target_argv);
    ctx.rdx = reinterpret_cast<uint64_t>(target_envp);
    ctx.cs  = 0x20 | 3;
    ctx.ds  = 0x18 | 3;
    ctx.ss  = 0x18 | 3;
    ctx.fs  = reinterpret_cast<uint64_t>(uthread);
    ctx.rsp = ctx.rbp = reinterpret_cast<uint64_t>(stack_zone) + 0x1000;
    ctx.rflags        = 0x200;
    // TODO: INSECURE! This allows all programs IOPORT access!
    ctx.rflags |= 0x3000;
#else
    ctx.eip = entry;
    ctx.cs  = 0x18 | 3;
    ctx.ds  = 0x20 | 3;
    ctx.ss  = 0x20 | 3;
    ctx.gs  = reinterpret_cast<uint32_t>(uthread);
    ctx.esp = ctx.ebp = (reinterpret_cast<addr_t>(stack_zone) + 0x1000) & ~15UL;
    // Arguments are passed on the stack
    uint32_t* stack = reinterpret_cast<uint32_t*>(ctx.esp);
    stack[-1]       = reinterpret_cast<uint32_t>(target_envp);
    stack[-2]       = reinterpret_cast<uint32_t>(target_argv);
    stack[-3]       = argc;
    ctx.esp -= 16;
    ctx.eflags = 0x200;
    ctx.eflags |= 0x3000;
#endif
    ctx.kernel_stack = CPU::X86::TSS::get_stack();
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

Thread* create_kernel_thread(Process* p, void (*entry_point)(void*), void* data)
{
    Thread* thread = new Thread(p);
    libcxx::memset(&thread->tcontext, 0, sizeof(thread->tcontext));
    addr_t stack =
        reinterpret_cast<addr_t>(new uint8_t[0x1000] + 0x1000) & ~15UL;
#ifdef X86_64
    thread->tcontext.rdi    = reinterpret_cast<addr_t>(data);
    thread->tcontext.rip    = reinterpret_cast<addr_t>(entry_point);
    thread->tcontext.rbp    = reinterpret_cast<addr_t>(stack);
    thread->tcontext.rsp    = reinterpret_cast<addr_t>(stack);
    thread->tcontext.cs     = 0x8;
    thread->tcontext.ds     = 0x10;
    thread->tcontext.ss     = 0x10;
    thread->tcontext.rflags = 0x200;
#else
    thread->tcontext.eip    = reinterpret_cast<addr_t>(entry_point);
    thread->tcontext.ebp    = reinterpret_cast<addr_t>(stack);
    thread->tcontext.esp    = reinterpret_cast<addr_t>(stack);
    thread->tcontext.cs     = 0x8;
    thread->tcontext.ds     = 0x10;
    thread->tcontext.ss     = 0x10;
    thread->tcontext.eflags = 0x200;
    uint32_t* stack_ptr     = reinterpret_cast<uint32_t*>(thread->tcontext.esp);
    stack_ptr[-1]           = reinterpret_cast<uint32_t>(data);
    thread->tcontext.esp -= 8;
#endif
    thread->tcontext.kernel_stack = stack;
    return thread;
}

extern "C" void load_register_state(struct InterruptContext* ctx);

void load_registers(struct ThreadContext& tcontext)
{
    struct InterruptContext ctx;
    load_context(&ctx, &tcontext);
    load_register_state(&ctx);
}
