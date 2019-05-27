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

void save_context(interrupt_context* ctx, struct thread_context* tcontext)
{
    encode_tcontext(ctx, tcontext);
}

void load_context(interrupt_context* ctx, struct thread_context* tcontext)
{
    decode_tcontext(ctx, tcontext);
    set_stack(tcontext->kernel_stack);
    set_thread_base(tcontext);
}

bool thread::load(addr_t binary, int argc, const char* argv[], int envc,
                  const char* envp[], struct thread_context& ctx)
{
    auto [success, entry] = elf::load(binary);

    /*
     * elf::load returns a pair. The first parameter (bool) indicates status,
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

    libcxx::pair<bool, addr_t> argv_zone;
    libcxx::pair<bool, addr_t> envp_zone;
    libcxx::pair<bool, addr_t> stack_zone;
    libcxx::pair<bool, addr_t> sigreturn_zone;
    libcxx::pair<bool, addr_t> tls_zone;

    size_t tls_raw_size = ROUND_UP(parent->tls_memsz, parent->tls_alignment);
    size_t tls_size     = tls_raw_size + sizeof(struct uthread);
    tls_size            = ROUND_UP(tls_size, parent->tls_alignment);

    if ((argv_zone = parent->vma->allocate(USER_START, argv_size)).first) {
        log::printk(log::log_level::DEBUG, "argv located at %p\n",
                    argv_zone.second);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate argv\n");
        return false;
    }
    if ((envp_zone = parent->vma->allocate(USER_START, envp_size)).first) {
        log::printk(log::log_level::DEBUG, "envp located at %p\n",
                    envp_zone.second);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate envp\n");
        return false;
    }
    if ((stack_zone = parent->vma->allocate_reverse(USER_START, 0x1000))
            .first) {
        log::printk(log::log_level::DEBUG, "Stack located at %p\n",
                    stack_zone.second);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate stack\n");
        return false;
    }
    if ((sigreturn_zone = parent->vma->allocate(USER_START, 0x1000)).first) {
        log::printk(log::log_level::DEBUG, "Sigreturn page located at %p\n",
                    sigreturn_zone.second);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate sigreturn page\n");
        return false;
    }
    if ((tls_zone = parent->vma->allocate(USER_START, tls_size)).first) {
        log::printk(log::log_level::DEBUG, "TLS copy located at %p\n",
                    tls_zone.second);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate TLS copy\n");
        return false;
    }
    memory::virt::map_range(argv_zone.second, argv_size,
                            PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    memory::virt::map_range(envp_zone.second, envp_size,
                            PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    memory::virt::map_range(stack_zone.second, 0x1000,
                            PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    memory::virt::map_range(sigreturn_zone.second, 0x1000,
                            PAGE_USER | PAGE_WRITABLE);
    memory::virt::map_range(tls_zone.second, tls_size,
                            PAGE_USER | PAGE_NX | PAGE_WRITABLE);

    this->parent->sigreturn = sigreturn_zone.second;

    // Copy in sigreturn trampoline code
    libcxx::memcpy(reinterpret_cast<void*>(sigreturn_zone.second),
                   signal_return_location, 0x1000);

    // Make it unwritable
    memory::virt::protect(sigreturn_zone.second, PAGE_USER);

    // Copy TLS data into thread specific data
    libcxx::memcpy((void*)tls_zone.second, (void*)parent->tls_base,
                   parent->tls_filesz);
    libcxx::memset((void*)(tls_zone.second + parent->tls_filesz), 0,
                   parent->tls_memsz - parent->tls_filesz);

    struct uthread* uthread =
        reinterpret_cast<struct uthread*>(tls_zone.second + tls_raw_size);
    uthread->self = uthread;

    char* target       = reinterpret_cast<char*>(argv_zone.second +
                                           (sizeof(char*) * (argc + 1)));
    char** target_argv = reinterpret_cast<char**>(argv_zone.second);
    for (int i = 0; i < argc; i++) {
        libcxx::strcpy(target, argv[i]);
        target_argv[i] = target;
        target += libcxx::strlen(argv[i]) + 1;
    }
    target_argv[argc]  = 0;
    target             = reinterpret_cast<char*>(envp_zone.second +
                                     (sizeof(char*) * (envc + 1)));
    char** target_envp = reinterpret_cast<char**>(envp_zone.second);
    for (int i = 0; i < envc; i++) {
        libcxx::strcpy(target, envp[i]);
        target_envp[i] = target;
        target += libcxx::strlen(envp[i]) + 1;
    }
    target_envp[envc] = 0;
    libcxx::memset(reinterpret_cast<void*>(stack_zone.second), 0, 0x1000);

    libcxx::memset(&ctx, 0, sizeof(ctx));
    ctx.rip = entry;
    ctx.rdi = argc;
    ctx.rsi = reinterpret_cast<uint64_t>(target_argv);
    ctx.rdx = reinterpret_cast<uint64_t>(target_envp);
    ctx.cs  = 0x20 | 3;
    ctx.ds  = 0x18 | 3;
    ctx.ss  = 0x18 | 3;
    ctx.fs  = reinterpret_cast<uint64_t>(uthread);
    ctx.rsp = ctx.rbp = reinterpret_cast<uint64_t>(stack_zone.second) + 0x1000;
    ctx.rflags        = 0x200;
    // TODO: INSECURE! This allows all programs IOPORT access!
    ctx.rflags |= 0x3000;
    ctx.kernel_stack = cpu::x86_64::TSS::get_stack();
    return true;
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
    thread* kthread = new thread(p);
    libcxx::memset(&kthread->tcontext, 0, sizeof(kthread->tcontext));
    addr_t stack =
        reinterpret_cast<addr_t>(new uint8_t[0xF000] + 0xF000) & ~15UL;
    kthread->tcontext.rdi          = reinterpret_cast<addr_t>(data);
    kthread->tcontext.rip          = reinterpret_cast<addr_t>(entry_point);
    kthread->tcontext.rbp          = reinterpret_cast<addr_t>(stack);
    kthread->tcontext.rsp          = reinterpret_cast<addr_t>(stack);
    kthread->tcontext.cs           = 0x8;
    kthread->tcontext.ds           = 0x10;
    kthread->tcontext.ss           = 0x10;
    kthread->tcontext.rflags       = 0x200;
    kthread->tcontext.kernel_stack = stack;
    return kthread;
}

extern "C" void load_register_state(struct interrupt_context* ctx);

void load_registers(struct thread_context& tcontext)
{
    struct interrupt_context ctx;
    load_context(&ctx, &tcontext);
    load_register_state(&ctx);
}
