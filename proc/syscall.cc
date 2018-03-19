#include <cpu/interrupt.h>
#include <errno.h>
#include <kernel.h>
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

static void sys_exit(int val)
{
    Log::printk(Log::DEBUG, "[sys_exit] = %d\n", val);
    Scheduler::remove(Scheduler::get_current_thread());
    for (;;) {
        __asm__("hlt");
    }
}

static void* syscall_table[256];

static void handler(int, void*, struct interrupt_ctx* ctx)
{
    Log::printk(Log::DEBUG,
                "Received system call %d, %llX %llX %llX %llX %llX\n", ctx->rax,
                ctx->rdi, ctx->rsi, ctx->rdx, ctx->rcx, ctx->r8);
    if (!syscall_table[ctx->rax]) {
        Log::printk(Log::ERROR, "Received invalid syscall #%d\n", ctx->rax);
        ctx->rax = -ENOSYS;
        return;
    }
    int (*func)(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) =
        (int (*)(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
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
    syscall_table[SYS_exit] = reinterpret_cast<void*>(sys_exit);
    Interrupt::register_handler(0x80, handler_data);
}
}
