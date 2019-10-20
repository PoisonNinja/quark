#include <arch/mm/layout.h>
#include <errno.h>
#include <fs/poll.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/module.h>
#include <lib/string.h>
#include <mm/virtual.h>
#include <proc/sched.h>
#include <proc/syscall.h>

void* syscall_table[256];

namespace
{
libcxx::intrusive_ptr<filesystem::descriptor> get_start(const char* path)
{
    libcxx::intrusive_ptr<filesystem::descriptor> start(nullptr);
    if (*path == '/') {
        start = scheduler::get_current_process()->get_root();
    } else {
        start = start = scheduler::get_current_process()->get_cwd();
    }
    return start;
}
} // namespace

namespace syscall
{
static long sys_read(int fd, void* buffer, size_t count)
{
    log::printk(log::log_level::DEBUG, "[sys_read] = %d, %p, %p\n", fd, buffer,
                count);
    if (!scheduler::get_current_process()->get_desc(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->get_desc(fd)->read(
        static_cast<uint8_t*>(buffer), count);
}

static long sys_write(int fd, const void* buffer, size_t count)
{
    log::printk(log::log_level::DEBUG, "[sys_write] = %d, %p, %X\n", fd, buffer,
                count);
    if (!scheduler::get_current_process()->get_desc(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->get_desc(fd)->write(
        static_cast<const uint8_t*>(buffer), count);
}

static long sys_open(const char* path, int flags, mode_t mode)
{
    log::printk(log::log_level::DEBUG, "[sys_open] = %s, %X, %X\n", path, flags,
                mode);
    auto [err, file] = get_start(path)->open(path, flags, mode);
    if (!file) {
        return -ENOENT;
    }
    int ret = scheduler::get_current_process()->install_desc(file);
    return ret;
}

static long sys_close(int fd)
{
    log::printk(log::log_level::DEBUG, "[sys_close] = %d\n", fd);
    if (!scheduler::get_current_process()->remove_desc(fd)) {
        return -1;
    } else {
        return 0;
    }
}

static long sys_stat(const char* path, struct filesystem::stat* st)
{
    log::printk(log::log_level::DEBUG, "[sys_stat] = %s, %p\n", path, st);
    auto [err, file] = get_start(path)->open(path, 0, 0);
    if (!file) {
        return -EBADF;
    }
    return file->stat(st);
}

static long sys_fstat(int fd, struct filesystem::stat* st)
{
    log::printk(log::log_level::DEBUG, "[sys_fstat] = %d, %p\n", fd, st);
    if (!scheduler::get_current_process()->get_desc(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->get_desc(fd)->stat(st);
}

static long sys_poll(struct filesystem::pollfd* fds, filesystem::nfds_t nfds,
                     int timeout)
{
    log::printk(log::log_level::DEBUG, "[sys_poll]: %p, %zu, %d\n", fds, nfds,
                timeout);
    filesystem::poll_table poller(fds, nfds);
    return poller.poll(timeout);
}

static long sys_lseek(int fd, off_t offset, int whence)
{
    log::printk(log::log_level::DEBUG, "[sys_lseek] = %d, %llu, %d\n", fd,
                offset, whence);
    if (!scheduler::get_current_process()->get_desc(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->get_desc(fd)->lseek(offset,
                                                                 whence);
}

static void* sys_mmap(struct mmap_wrapper* mmap_data)
{
    log::printk(log::log_level::DEBUG,
                "[sys_mmap] = %p, %p, %zX, %u, %u, %d, %zX\n", mmap_data,
                mmap_data->addr, mmap_data->length, mmap_data->prot,
                mmap_data->flags, mmap_data->fd, mmap_data->offset);
    return scheduler::get_current_process()->mmap(
        reinterpret_cast<addr_t>(mmap_data->addr), mmap_data->length,
        mmap_data->prot, mmap_data->flags, nullptr, mmap_data->offset);
}

static long sys_sigaction(int signum, struct sigaction* act,
                          struct sigaction* oldact)
{
    log::printk(log::log_level::DEBUG, "[sys_sigaction]: %d %p %p\n", signum,
                act, oldact);
    return scheduler::get_current_process()->sigaction(signum, act, oldact);
}

static long sys_sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
    log::printk(log::log_level::DEBUG, "[sys_sigprocmask] %d %p %p\n", how, set,
                oldset);
    return scheduler::get_current_thread()->sigprocmask(how, set, oldset);
}

static void sys_sigreturn(ucontext_t* uctx)
{
    log::printk(log::log_level::DEBUG, "[sys_return] %p\n", uctx);
    scheduler::get_current_thread()->handle_sigreturn(uctx);
}

static long sys_ioctl(int fd, unsigned long request, char* argp)
{
    log::printk(log::log_level::DEBUG, "[sys_ioctl] = %d, 0x%lX, %p\n", fd,
                request, argp);
    if (!scheduler::get_current_process()->get_desc(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->get_desc(fd)->ioctl(request, argp);
}

static long sys_dup(int oldfd)
{
    log::printk(log::log_level::DEBUG, "[sys_dup] = %d\n", oldfd);
    auto desc = scheduler::get_current_process()->get_desc(oldfd);
    if (!desc) {
        return -EBADF;
    }
    return scheduler::get_current_process()->install_desc(desc);
}

static long sys_dup2(int oldfd, int newfd)
{
    log::printk(log::log_level::DEBUG, "[sys_dup2] = %d %d\n", oldfd, newfd);
    return scheduler::get_current_process()->copy_desc(oldfd, newfd);
}

static long sys_getpid()
{
    // Return process ID not thread ID
    log::printk(log::log_level::DEBUG, "[sys_getpid]\n");
    return scheduler::get_current_process()->get_pid();
}

static long sys_fork()
{
    log::printk(log::log_level::DEBUG, "[sys_fork]\n");
    process* child       = scheduler::get_current_process()->fork();
    thread* child_thread = child->create_thread();
    thread_context ctx   = scheduler::get_current_thread()->get_context();
    // Child process gets 0 returned from fork
    ctx.rax = 0;
    child_thread->set_context(ctx);
    scheduler::insert(child_thread);
    return child->get_pid();
}

static long sys_execve(const char* path, const char* old_argv[],
                       const char* old_envp[])
{
    log::printk(log::log_level::DEBUG, "[sys_execve]: %s %p %p\n", path,
                old_argv, old_envp);
    size_t argc = 0, envc = 0;
    while (old_argv[argc]) {
        argc++;
    }
    while (old_envp[envc]) {
        envc++;
    }
    const char** argv = new const char*[argc];
    for (size_t i = 0; i < argc; i++) {
        argv[i] = new char[libcxx::strlen(old_argv[i])];
        libcxx::strcpy(const_cast<char*>(argv[i]), old_argv[i]);
    }
    const char** envp = new const char*[envc];
    for (size_t i = 0; i < envc; i++) {
        envp[i] = new char[libcxx::strlen(old_envp[i])];
        libcxx::strcpy(const_cast<char*>(envp[i]), old_envp[i]);
    }
    auto [err, file] = get_start(path)->open(path, 0, 0);
    if (!file) {
        delete[] envp;
        delete[] argv;
        return -ENOENT;
    }
    struct thread_context ctx;
    if (scheduler::get_current_process()->load(file, argc, argv, envc, envp,
                                               ctx)) {
        log::printk(log::log_level::ERROR, "Failed to load thread state\n");
        return -ENOEXEC;
    }
    delete[] envp;
    delete[] argv;
    load_registers(ctx);
    __builtin_unreachable();
}

static void sys_exit(int val)
{
    log::printk(log::log_level::DEBUG, "[sys_exit] = %d\n", val);
    scheduler::get_current_thread()->exit(false, val);
}

static long sys_wait(pid_t pid, int* status, int options)
{
    log::printk(log::log_level::DEBUG, "[sys_wait] %lld %p %d\n", pid, status,
                options);
    return scheduler::get_current_process()->wait(pid, status, options);
}

static long sys_kill(pid_t pid, int signum)
{
    log::printk(log::log_level::DEBUG, "[sys_kill] %u %d\n", pid, signum);
    process* process = scheduler::find_process(pid);
    if (!process) {
        log::printk(log::log_level::WARNING,
                    "Failed to find process with PID %d\n", pid);
        return -ESRCH;
    }
    log::printk(log::log_level::DEBUG, "Found process %d\n",
                process->get_pid());
    process->send_signal(signum);
    return 0;
}

static long sys_chdir(const char* path)
{
    log::printk(log::log_level::DEBUG, "[sys_chdir] %s\n", path);
    auto [err, dir] =
        get_start(path)->open(path, filesystem::descriptor_flags::F_READ, 0);
    if (err) {
        return err;
    }
    scheduler::get_current_process()->set_cwd(dir);
    return 0;
}

static long sys_mkdir(const char* path, mode_t mode)
{
    log::printk(log::log_level::DEBUG, "[sys_mkdir] %s %X\n", path, mode);
    return get_start(path)->mkdir(path, mode);
}

static long sys_sigpending(sigset_t* set)
{
    log::printk(log::log_level::DEBUG, "[sys_sigpending] %p\n", set);
    return scheduler::get_current_thread()->sigpending(set);
}

static long sys_sigaltstack(const stack_t* ss, stack_t* oldss)
{
    log::printk(log::log_level::DEBUG, "[sys_sigaltstack] %p %p\n", ss, oldss);
    return scheduler::get_current_thread()->sigaltstack(ss, oldss);
}

static long sys_mknod(const char* path, mode_t mode, dev_t dev)
{
    log::printk(log::log_level::DEBUG, "[sys_mknod] %s %X %X\n", path, mode,
                dev);
    return get_start(path)->mknod(path, mode, dev);
}

static long sys_chroot(const char* path)
{
    log::printk(log::log_level::DEBUG, "[sys_chroot] %s\n", path);
    auto [err, dir] =
        get_start(path)->open(path, filesystem::descriptor_flags::F_READ, 0);
    if (err) {
        return err;
    }
    scheduler::get_current_process()->set_root(dir);
    return 0;
}

static long sys_mount(const char* source, const char* target,
                      const char* filesystemtype, unsigned long mountflags,
                      const void* data)
{
    log::printk(log::log_level::DEBUG, "[sys_mount] %s %s %s %llX %p\n", source,
                target, filesystemtype, mountflags, data);
    return get_start(source)->mount(source, target, filesystemtype, mountflags);
}

static long sys_init_module(void* module_image, unsigned long len,
                            const char* param_values)
{
    log::printk(log::log_level::DEBUG, "[sys_init_module] %p %llX %p\n",
                module_image, len, param_values);
    return load_module(module_image);
}

static long sys_delete_module(const char* name, int flags)
{
    log::printk(log::log_level::DEBUG, "[sys_delete_module] %s %X\n", name,
                flags);
    return unload_module(name);
}

void init()
{
    syscall_table[SYS_read]        = reinterpret_cast<void*>(sys_read);
    syscall_table[SYS_write]       = reinterpret_cast<void*>(sys_write);
    syscall_table[SYS_open]        = reinterpret_cast<void*>(sys_open);
    syscall_table[SYS_close]       = reinterpret_cast<void*>(sys_close);
    syscall_table[SYS_stat]        = reinterpret_cast<void*>(sys_stat);
    syscall_table[SYS_fstat]       = reinterpret_cast<void*>(sys_fstat);
    syscall_table[SYS_poll]        = reinterpret_cast<void*>(sys_poll);
    syscall_table[SYS_lseek]       = reinterpret_cast<void*>(sys_lseek);
    syscall_table[SYS_mmap]        = reinterpret_cast<void*>(sys_mmap);
    syscall_table[SYS_sigaction]   = reinterpret_cast<void*>(sys_sigaction);
    syscall_table[SYS_sigprocmask] = reinterpret_cast<void*>(sys_sigprocmask);
    syscall_table[SYS_sigreturn]   = reinterpret_cast<void*>(sys_sigreturn);
    syscall_table[SYS_ioctl]       = reinterpret_cast<void*>(sys_ioctl);
    syscall_table[SYS_dup]         = reinterpret_cast<void*>(sys_dup),
    syscall_table[SYS_dup2]        = reinterpret_cast<void*>(sys_dup2),
    syscall_table[SYS_getpid]      = reinterpret_cast<void*>(sys_getpid);
    syscall_table[SYS_fork]        = reinterpret_cast<void*>(sys_fork);
    syscall_table[SYS_execve]      = reinterpret_cast<void*>(sys_execve);
    syscall_table[SYS_exit]        = reinterpret_cast<void*>(sys_exit);
    syscall_table[SYS_wait]        = reinterpret_cast<void*>(sys_wait);
    syscall_table[SYS_kill]        = reinterpret_cast<void*>(sys_kill);
    syscall_table[SYS_chdir]       = reinterpret_cast<void*>(sys_chdir);
    syscall_table[SYS_mkdir]       = reinterpret_cast<void*>(sys_mkdir);
    syscall_table[SYS_sigpending]  = reinterpret_cast<void*>(sys_sigpending);
    syscall_table[SYS_sigaltstack] = reinterpret_cast<void*>(sys_sigaltstack);
    syscall_table[SYS_mknod]       = reinterpret_cast<void*>(sys_mknod);
    syscall_table[SYS_chroot]      = reinterpret_cast<void*>(sys_chroot);
    syscall_table[SYS_mount]       = reinterpret_cast<void*>(sys_mount);
    syscall_table[SYS_init_module] = reinterpret_cast<void*>(sys_init_module);
    syscall_table[SYS_delete_module] =
        reinterpret_cast<void*>(sys_delete_module);
}
} // namespace syscall
