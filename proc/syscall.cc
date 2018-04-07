#include <arch/mm/layout.h>
#include <cpu/interrupt.h>
#include <errno.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/virtual.h>
#include <proc/sched.h>
#include <proc/syscall.h>

namespace Syscall
{
static ssize_t sys_read(int fd, const void* buffer, size_t count)
{
    Log::printk(Log::DEBUG, "[sys_read] = %d, %p, %llX\n", fd, buffer, count);
    if (!Scheduler::get_current_process()->fds[fd]) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->fds[fd]->read(
        const_cast<uint8_t*>(static_cast<const uint8_t*>(buffer)), count);
}
static ssize_t sys_write(int fd, const void* buffer, size_t count)
{
    Log::printk(Log::DEBUG, "[sys_write] = %d, %p, %llX\n", fd, buffer, count);
    if (!Scheduler::get_current_process()->fds[fd]) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->fds[fd]->write(
        const_cast<uint8_t*>(static_cast<const uint8_t*>(buffer)), count);
}

static int sys_open(const char* path, int flags, mode_t mode)
{
    Log::printk(Log::DEBUG, "[sys_open] = %s, %X, %X\n", path, flags, mode);
    Ref<Filesystem::Descriptor> start(nullptr);
    if (*path == '/') {
        start = Scheduler::get_current_process()->root;
    } else {
        start = start = Scheduler::get_current_process()->cwd;
    }
    Ref<Filesystem::Descriptor> file = start->open(path, flags, mode);
    int ret = Scheduler::get_current_process()->fds.add(file);
    if (Scheduler::get_current_process()->fds[ret] != file) {
        Log::printk(Log::ERROR,
                    "WTF, someone is lying about the file descriptor...\n");
        return -1;
    }
    return ret;
}

static int sys_close(int fd)
{
    Log::printk(Log::DEBUG, "[sys_close] = %d\n", fd);
    if (!Scheduler::get_current_process()->fds.remove(fd)) {
        return -1;
    } else {
        return 0;
    }
}

static int sys_stat(const char* path, struct Filesystem::stat* st)
{
    Log::printk(Log::DEBUG, "[sys_fstat] = %s, %p\n", path, st);
    Ref<Filesystem::Descriptor> start(nullptr);
    if (*path == '/') {
        start = Scheduler::get_current_process()->root;
    } else {
        start = start = Scheduler::get_current_process()->cwd;
    }
    Ref<Filesystem::Descriptor> file = start->open(path, 0, 0);
    return file->stat(st);
}

static int sys_fstat(int fd, struct Filesystem::stat* st)
{
    Log::printk(Log::DEBUG, "[sys_fstat] = %d, %p\n", fd, st);
    if (!Scheduler::get_current_process()->fds[fd]) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->fds[fd]->stat(st);
}

static off_t sys_lseek(int fd, off_t offset, int whence)
{
    Log::printk(Log::DEBUG, "[sys_lseek] = %d, %llu, %d\n", fd, offset, whence);
    if (!Scheduler::get_current_process()->fds[fd]) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->fds[fd]->lseek(offset, whence);
}

static void* sys_mmap(struct mmap_wrapper* mmap_data)
{
    Log::printk(Log::DEBUG, "[sys_mmap] = %p, %p, %llX, %u, %u, %d, %llX\n",
                mmap_data, mmap_data->addr, mmap_data->length, mmap_data->prot,
                mmap_data->flags, mmap_data->fd, mmap_data->offset);
    if (mmap_data->flags & MAP_SHARED) {
        Log::printk(Log::WARNING, "[sys_mmap] Userspace mmap requested "
                                  "MMAP_SHARED, you should probably implement "
                                  "this\n");
        return MAP_FAILED;
    }
    if (!(mmap_data->flags & MAP_ANONYMOUS)) {
        Log::printk(Log::WARNING, "[sys_mmap] Userspace mmap requested file "
                                  "mapping, you should probably implement "
                                  "this\n");
        return MAP_FAILED;
    }
    if (!(mmap_data->flags & MAP_FIXED)) {
        Log::printk(Log::DEBUG, "[sys_mmap] Kernel selecting mapping\n");
        addr_t placement;
        bool ret = false;
        ret = Scheduler::get_current_process()->sections->locate_range(
            placement,
            (mmap_data->addr) ? reinterpret_cast<addr_t>(mmap_data->addr) :
                                USER_START,
            mmap_data->length);
        if (!ret) {
            Log::printk(Log::WARNING,
                        "[sys_mmap] Failed to allocate area for mmap\n");
            return MAP_FAILED;
        }
        Log::printk(Log::DEBUG, "[sys_mmap] Selected %p\n", placement);
        Scheduler::get_current_process()->sections->add_section(
            placement, mmap_data->length);
        int flags = Memory::Virtual::prot_to_flags(mmap_data->prot);
        for (addr_t base = placement; base < placement + mmap_data->length;
             base += Memory::Virtual::PAGE_SIZE) {
            Memory::Virtual::map(base, flags);
        }
        return reinterpret_cast<void*>(placement);
    } else {
        Log::printk(Log::WARNING, "[sys_mmap] Userspace mmap requested "
                                  "MAP_FIXED, you should probably implement "
                                  "it\n");
        return MAP_FAILED;
    }
}

static pid_t sys_fork()
{
    addr_t cloned = Memory::Virtual::fork();
    Process* child = new Process(Scheduler::get_current_process());
    child->fds = Scheduler::get_current_process()->fds;
    child->root = Scheduler::get_current_process()->root;
    child->cwd = Scheduler::get_current_process()->cwd;
    child->address_space = cloned;
    Thread* thread = new Thread(child);
    String::memcpy(&thread->cpu_ctx, &Scheduler::get_current_thread()->cpu_ctx,
                   sizeof(thread->cpu_ctx));
    thread->cpu_ctx.rax = 0;
    thread->kernel_stack = (addr_t) new uint8_t[0x1000] + 0x1000;
    Scheduler::insert(thread);
    return child->pid;
}

static void sys_exit(int val)
{
    Log::printk(Log::DEBUG, "[sys_exit] = %d\n", val);
    Scheduler::remove(Scheduler::get_current_thread());
    for (;;) {
        __asm__("hlt");
    }
}

static void* syscall_table[256];

static void handler(int, void*, struct InterruptContext* ctx)
{
    Scheduler::get_current_thread()->save_context(ctx);
    Log::printk(
        Log::DEBUG,
        "Received system call %d from PID %d, %llX %llX %llX %llX %llX\n",
        ctx->rax, Scheduler::get_current_thread()->tid, ctx->rdi, ctx->rsi,
        ctx->rdx, ctx->rcx, ctx->r8);
    if (!syscall_table[ctx->rax]) {
        Log::printk(Log::ERROR, "Received invalid syscall #%d\n", ctx->rax);
        ctx->rax = -ENOSYS;
        return;
    }
    uint64_t (*func)(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e) =
        (uint64_t(*)(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                     uint64_t e))syscall_table[ctx->rax];
    ctx->rax = func(ctx->rdi, ctx->rsi, ctx->rdx, ctx->rcx, ctx->r8);
}

static struct Interrupt::Handler handler_data(handler, "syscall",
                                              &handler_data);

void init()
{
    syscall_table[SYS_read] = reinterpret_cast<void*>(sys_read);
    syscall_table[SYS_write] = reinterpret_cast<void*>(sys_write);
    syscall_table[SYS_open] = reinterpret_cast<void*>(sys_open);
    syscall_table[SYS_close] = reinterpret_cast<void*>(sys_close);
    syscall_table[SYS_stat] = reinterpret_cast<void*>(sys_stat);
    syscall_table[SYS_fstat] = reinterpret_cast<void*>(sys_fstat);
    syscall_table[SYS_lseek] = reinterpret_cast<void*>(sys_lseek);
    syscall_table[SYS_mmap] = reinterpret_cast<void*>(sys_mmap);
    syscall_table[SYS_fork] = reinterpret_cast<void*>(sys_fork);
    syscall_table[SYS_exit] = reinterpret_cast<void*>(sys_exit);
    Interrupt::register_handler(0x80, handler_data);
}
}
