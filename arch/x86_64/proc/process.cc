#include <arch/mm/layout.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/elf.h>
#include <proc/process.h>

extern "C" void signal_return();
void* signal_return_location = (void*)&signal_return;

bool Process::load(addr_t binary, int argc, const char* argv[], int envc,
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
    addr_t sigreturn_zone;
    if (sections->locate_range(argv_zone, USER_START, argv_size)) {
        Log::printk(Log::DEBUG, "argv located at %p\n", argv_zone);
        sections->add_section(argv_zone, argv_size);
    } else {
        Log::printk(Log::ERROR, "Failed to locate argv\n");
        return false;
    }
    if (sections->locate_range(envp_zone, USER_START, envp_size)) {
        Log::printk(Log::DEBUG, "envp located at %p\n", envp_zone);
        sections->add_section(envp_zone, envp_size);
    } else {
        Log::printk(Log::ERROR, "Failed to locate envp\n");
        return false;
    }
    if (sections->locate_range(stack_zone, USER_START, 0x1000)) {
        Log::printk(Log::DEBUG, "Stack located at %p\n", stack_zone);
        sections->add_section(stack_zone, 0x1000);
    } else {
        Log::printk(Log::ERROR, "Failed to locate stack\n");
        return false;
    }
    if (sections->locate_range(sigreturn_zone, USER_START, 0x1000)) {
        Log::printk(Log::DEBUG, "Sigreturn page located at %p\n",
                    sigreturn_zone);
        sections->add_section(sigreturn_zone, 0x1000);
    } else {
        Log::printk(Log::ERROR, "Failed to locate sigreturn page\n");
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

    this->sigreturn = sigreturn_zone;

    // Copy in sigreturn trampoline code
    String::memcpy((void*)sigreturn_zone, signal_return_location, 0x1000);

    // Make it unwritable
    Memory::Virtual::protect(sigreturn_zone, PAGE_USER);

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
    ctx.rip = entry;
    ctx.rdi = argc;
    ctx.rsi = reinterpret_cast<uint64_t>(target_argv);
    ctx.rdx = reinterpret_cast<uint64_t>(target_envp);
    ctx.cs = 0x20 | 3;
    ctx.ds = 0x18 | 3;
    ctx.ss = 0x18 | 3;
    ctx.fs = 0xDEADBEEF;
    ctx.rsp = ctx.rbp = reinterpret_cast<uint64_t>(stack_zone) + 0x1000;
    ctx.rflags = 0x200;
    return true;
}