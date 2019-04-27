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
    if (!scheduler::get_current_process()->fds.get(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->fds.get(fd)->read(
        static_cast<uint8_t*>(buffer), count);
}

static long sys_write(int fd, const void* buffer, size_t count)
{
    log::printk(log::log_level::DEBUG, "[sys_write] = %d, %p, %X\n", fd, buffer,
                count);
    if (!scheduler::get_current_process()->fds.get(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->fds.get(fd)->write(
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
    int ret = scheduler::get_current_process()->fds.add(file);
    if (scheduler::get_current_process()->fds.get(ret) != file) {
        log::printk(log::log_level::ERROR,
                    "WTF, someone is lying about the file descriptor...\n");
        return -1;
    }
    return ret;
}

static long sys_close(int fd)
{
    log::printk(log::log_level::DEBUG, "[sys_close] = %d\n", fd);
    if (!scheduler::get_current_process()->fds.remove(fd)) {
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
    if (!scheduler::get_current_process()->fds.get(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->fds.get(fd)->stat(st);
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
    if (!scheduler::get_current_process()->fds.get(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->fds.get(fd)->lseek(offset, whence);
}

static void* sys_mmap(struct mmap_wrapper* mmap_data)
{
    log::printk(log::log_level::DEBUG,
                "[sys_mmap] = %p, %p, %zX, %u, %u, %d, %zX\n", mmap_data,
                mmap_data->addr, mmap_data->length, mmap_data->prot,
                mmap_data->flags, mmap_data->fd, mmap_data->offset);
    if (mmap_data->flags & MAP_SHARED) {
        log::printk(log::log_level::WARNING,
                    "[sys_mmap] Userspace mmap requested "
                    "MMAP_SHARED, you should probably implement"
                    "this\n");
        return MAP_FAILED;
    }
    if (!(mmap_data->flags & MAP_ANONYMOUS)) {
        log::printk(log::log_level::WARNING,
                    "[sys_mmap] Userspace mmap requested file "
                    "mapping, you should probably implement "
                    "this\n");
        return MAP_FAILED;
    }
    if (!(mmap_data->flags & MAP_FIXED)) {
        log::printk(log::log_level::DEBUG,
                    "[sys_mmap] Kernel selecting mapping\n");
        auto [ret, placement] =
            scheduler::get_current_process()->vma->locate_range(
                (mmap_data->addr) ? reinterpret_cast<addr_t>(mmap_data->addr)
                                  : USER_START,
                mmap_data->length);
        if (!ret) {
            log::printk(log::log_level::WARNING,
                        "[sys_mmap] Failed to allocate area for mmap\n");
            return MAP_FAILED;
        }
        log::printk(log::log_level::DEBUG, "[sys_mmap] Selected %p\n",
                    placement);
        scheduler::get_current_process()->vma->add_vmregion(placement,
                                                            mmap_data->length);
        int flags = memory::virt::prot_to_flags(mmap_data->prot);
        memory::virt::map_range(placement, mmap_data->length, flags);
        return reinterpret_cast<void*>(placement);
    } else {
        log::printk(log::log_level::WARNING,
                    "[sys_mmap] Userspace mmap requested "
                    "MAP_FIXED, you should probably implement "
                    "it\n");
        return MAP_FAILED;
    }
}

static long sys_sigaction(int signum, struct sigaction* act,
                          struct sigaction* oldact)
{
    log::printk(log::log_level::DEBUG, "[sys_sigaction]: %d %p %p\n", signum,
                act, oldact);
    process* current = scheduler::get_current_process();
    if (oldact) {
        libcxx::memcpy(oldact, &current->signal_actions[signum],
                       sizeof(*oldact));
    }
    libcxx::memcpy(&current->signal_actions[signum], act, sizeof(*act));
    return 0;
}

static long sys_sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
    log::printk(log::log_level::DEBUG, "[sys_sigprocmask] %d %p %p\n", how, set,
                oldset);
    if (oldset) {
        *oldset = scheduler::get_current_thread()->signal_mask;
    }
    if (how == SIG_SETMASK) {
        scheduler::get_current_thread()->signal_mask = *set;
    } else if (how == SIG_BLOCK) {
        signal::sigorset(&scheduler::get_current_thread()->signal_mask, set);
    } else if (how == SIG_UNBLOCK) {
        sigset_t dup = *set;
        signal::signotset(&dup, &dup);
        signal::sigandset(&scheduler::get_current_thread()->signal_mask, &dup);
    } else {
        return -EINVAL;
    }
    return 0;
}

static void sys_sigreturn(ucontext_t* uctx)
{
    log::printk(log::log_level::DEBUG, "[sys_return] %p\n", uctx);
    thread_context tctx;
    /*
     * The syscall handler saves the userspace context, so we copy it into
     * tctx to get certain registers (DS, ES, SS) preloaded for us. The
     * rest of the state will get overriden by the stored mcontext
     */
    libcxx::memcpy(&tctx, &scheduler::get_current_thread()->tcontext,
                   sizeof(tctx));
    // Unset on_stack
    if (scheduler::get_current_thread()->signal_stack.ss_flags & SS_ONSTACK) {
        scheduler::get_current_thread()->signal_stack.ss_flags &= ~SS_ONSTACK;
    }
    // Restore signal mask
    scheduler::get_current_thread()->signal_mask = uctx->uc_sigmask;
    signal::decode_mcontext(&uctx->uc_mcontext, &tctx);
    load_registers(tctx);
}

static long sys_ioctl(int fd, unsigned long request, char* argp)
{
    log::printk(log::log_level::DEBUG, "[sys_ioctl] = %d, 0x%lX, %p\n", fd,
                request, argp);
    if (!scheduler::get_current_process()->fds.get(fd)) {
        return -EBADF;
    }
    return scheduler::get_current_process()->fds.get(fd)->ioctl(request, argp);
}

static long sys_dup(int oldfd)
{
    log::printk(log::log_level::DEBUG, "[sys_dup] = %d\n", oldfd);
    auto desc = scheduler::get_current_process()->fds.get(oldfd);
    if (!desc) {
        return -EBADF;
    }
    return scheduler::get_current_process()->fds.add(desc);
}

static long sys_dup2(int oldfd, int newfd)
{
    log::printk(log::log_level::DEBUG, "[sys_dup2] = %d %d\n", oldfd, newfd);
    return scheduler::get_current_process()->fds.copy(oldfd, newfd);
}

static long sys_getpid()
{
    // Return process ID not thread ID
    log::printk(log::log_level::DEBUG, "[sys_getpid]\n");
    return scheduler::get_current_process()->pid;
}

static long sys_fork()
{
    log::printk(log::log_level::DEBUG, "[sys_fork]\n");
    process* child       = scheduler::get_current_process()->fork();
    thread* child_thread = new thread(child);
    libcxx::memcpy(&child_thread->tcontext,
                   &scheduler::get_current_thread()->tcontext,
                   sizeof(child_thread->tcontext));
    // Child process gets 0 returned from fork
#ifdef X86_64
    child_thread->tcontext.rax = 0;
#else
    child_thread->tcontext.eax = 0;
#endif
    child_thread->tcontext.kernel_stack =
        reinterpret_cast<addr_t>(new uint8_t[0xF000] + 0xF000) & ~15UL;
    scheduler::insert(child_thread);
    return child->pid;
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
    struct filesystem::stat st;
    file->stat(&st);
    log::printk(log::log_level::DEBUG,
                "[sys_execve] binary has size of %zu bytes\n", st.st_size);
    uint8_t* raw = new uint8_t[st.st_size];
    file->read(raw, st.st_size);
    scheduler::get_current_process()->vma->reset();
    struct thread_context ctx;
    if (!scheduler::get_current_thread()->load(reinterpret_cast<addr_t>(raw),
                                               argc, argv, envc, envp, ctx)) {
        log::printk(log::log_level::ERROR, "Failed to load thread state\n");
        return -ENOEXEC;
    }
    delete[] raw;
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
    log::printk(log::log_level::INFO, "[sys_wait] %lld %p %d\n", pid, status,
                options);
    return scheduler::get_current_process()->wait(pid, status, options);
}

static long sys_kill(pid_t pid, int signum)
{
    log::printk(log::log_level::DEBUG, "[sys_kill] %u %d\n", pid, signum);
    process* process = scheduler::find_process(pid);
    if (!process) {
        return -ESRCH;
    }
    log::printk(log::log_level::DEBUG, "Found process at %p\n", process);
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
    if (!set) {
        return -EFAULT;
    }
    sigset_t pending = scheduler::get_current_thread()->signal_pending;
    /*
     * Signals that are both blocked and ignored are NOT added to the mask
     * of pending signals.
     *
     * However, signals that are merely blocked will be added
     */
    sigset_t blocked_and_ignored;
    // Get the list of blocked signals
    signal::sigorset(&blocked_and_ignored,
                     &scheduler::get_current_thread()->signal_mask);
    // Remove signals that are NOT ignored
    for (int i = 1; i < NSIGS; i++) {
        // TODO: Also check if SIG_DFL is set but the default action is to
        // ignore
        if (scheduler::get_current_process()->signal_actions[i].sa_handler !=
            SIG_IGN) {
            signal::sigdelset(&blocked_and_ignored, i);
        }
    }
    // Invert the set so that signals that are both blocked and ignored are
    // unset
    signal::signotset(&blocked_and_ignored, &blocked_and_ignored);
    // AND the two sets together to basically unset blocked and ignored
    signal::sigandset(&pending, &blocked_and_ignored);

    *set = pending;

    /*
     * TODO: Figure out what happens if a signal is handled right after this
     * system call returns.
     *
     * System calls will check for pending signals before returning and will
     * handle them before returning so it's possible that the set returned
     * still has the bit set even though the signal is already handled.
     */
    return 0;
}

static long sys_sigaltstack(const stack_t* ss, stack_t* oldss)
{
    log::printk(log::log_level::DEBUG, "[sys_sigaltstack] %p %p\n", ss, oldss);
    if (oldss) {
        libcxx::memcpy(oldss, &scheduler::get_current_thread()->signal_stack,
                       sizeof(*oldss));
    }
    if (ss) {
        libcxx::memcpy(&scheduler::get_current_thread()->signal_stack, ss,
                       sizeof(*ss));
    }
    return 0;
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
