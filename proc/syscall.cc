#include <arch/mm/layout.h>
#include <errno.h>
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
libcxx::intrusive_ptr<Filesystem::Descriptor> get_start(const char* path)
{
    libcxx::intrusive_ptr<Filesystem::Descriptor> start(nullptr);
    if (*path == '/') {
        start = Scheduler::get_current_process()->get_root();
    } else {
        start = start = Scheduler::get_current_process()->get_cwd();
    }
    return start;
}
} // namespace

namespace Syscall
{
static long sys_read(int fd, const void* buffer, size_t count)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_read] = %d, %p, %p\n", fd, buffer,
                count);
    if (!Scheduler::get_current_process()->get_dtable()->get(fd)) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->get_dtable()->get(fd)->read(
        const_cast<uint8_t*>(static_cast<const uint8_t*>(buffer)), count);
}

static long sys_write(int fd, const void* buffer, size_t count)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_write] = %d, %p, %X\n", fd, buffer,
                count);
    if (!Scheduler::get_current_process()->get_dtable()->get(fd)) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->get_dtable()->get(fd)->write(
        const_cast<uint8_t*>(static_cast<const uint8_t*>(buffer)), count);
}

static long sys_open(const char* path, int flags, mode_t mode)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_open] = %s, %X, %X\n", path, flags,
                mode);
    libcxx::intrusive_ptr<Filesystem::Descriptor> file =
        get_start(path)->open(path, flags, mode);
    if (!file) {
        return -ENOENT;
    }
    int ret = Scheduler::get_current_process()->get_dtable()->add(file);
    if (Scheduler::get_current_process()->get_dtable()->get(ret) != file) {
        Log::printk(Log::LogLevel::ERROR,
                    "WTF, someone is lying about the file descriptor...\n");
        return -1;
    }
    return ret;
}

static long sys_close(int fd)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_close] = %d\n", fd);
    if (!Scheduler::get_current_process()->get_dtable()->remove(fd)) {
        return -1;
    } else {
        return 0;
    }
}

static long sys_stat(const char* path, struct Filesystem::stat* st)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_fstat] = %s, %p\n", path, st);
    libcxx::intrusive_ptr<Filesystem::Descriptor> file =
        get_start(path)->open(path, 0, 0);
    return file->stat(st);
}

static long sys_fstat(int fd, struct Filesystem::stat* st)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_fstat] = %d, %p\n", fd, st);
    if (!Scheduler::get_current_process()->get_dtable()->get(fd)) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->get_dtable()->get(fd)->stat(st);
}

static long sys_lseek(int fd, off_t offset, int whence)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_lseek] = %d, %llu, %d\n", fd,
                offset, whence);
    if (!Scheduler::get_current_process()->get_dtable()->get(fd)) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->get_dtable()->get(fd)->lseek(
        offset, whence);
}

static void* sys_mmap(struct mmap_wrapper* mmap_data)
{
    Log::printk(Log::LogLevel::DEBUG,
                "[sys_mmap] = %p, %p, %zX, %u, %u, %d, %zX\n", mmap_data,
                mmap_data->addr, mmap_data->length, mmap_data->prot,
                mmap_data->flags, mmap_data->fd, mmap_data->offset);
    if (mmap_data->flags & MAP_SHARED) {
        Log::printk(Log::LogLevel::WARNING,
                    "[sys_mmap] Userspace mmap requested "
                    "MMAP_SHARED, you should probably implement"
                    "this\n");
        return MAP_FAILED;
    }
    if (!(mmap_data->flags & MAP_ANONYMOUS)) {
        Log::printk(Log::LogLevel::WARNING,
                    "[sys_mmap] Userspace mmap requested file "
                    "mapping, you should probably implement "
                    "this\n");
        return MAP_FAILED;
    }
    if (!(mmap_data->flags & MAP_FIXED)) {
        Log::printk(Log::LogLevel::DEBUG,
                    "[sys_mmap] Kernel selecting mapping\n");
        addr_t placement;
        bool ret = false;
        ret      = Scheduler::get_current_process()->sections->locate_range(
            placement,
            (mmap_data->addr) ? reinterpret_cast<addr_t>(mmap_data->addr)
                              : USER_START,
            mmap_data->length);
        if (!ret) {
            Log::printk(Log::LogLevel::WARNING,
                        "[sys_mmap] Failed to allocate area for mmap\n");
            return MAP_FAILED;
        }
        Log::printk(Log::LogLevel::DEBUG, "[sys_mmap] Selected %p\n",
                    placement);
        Scheduler::get_current_process()->sections->add_section(
            placement, mmap_data->length);
        int flags = Memory::Virtual::prot_to_flags(mmap_data->prot);
        Memory::Virtual::map_range(placement, mmap_data->length, flags);
        return reinterpret_cast<void*>(placement);
    } else {
        Log::printk(Log::LogLevel::WARNING,
                    "[sys_mmap] Userspace mmap requested "
                    "MAP_FIXED, you should probably implement "
                    "it\n");
        return MAP_FAILED;
    }
}

static long sys_sigaction(int signum, struct sigaction* act,
                          struct sigaction* oldact)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_sigaction]: %d %p %p\n", signum,
                act, oldact);
    Process* current = Scheduler::get_current_process();
    if (oldact) {
        String::memcpy(oldact, &current->signal_actions[signum],
                       sizeof(*oldact));
    }
    String::memcpy(&current->signal_actions[signum], act, sizeof(*act));
    return 0;
}

static long sys_sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_sigprocmask] %d %p %p\n", how, set,
                oldset);
    if (oldset) {
        *oldset = Scheduler::get_current_thread()->signal_mask;
    }
    if (how == SIG_SETMASK) {
        Scheduler::get_current_thread()->signal_mask = *set;
    } else if (how == SIG_BLOCK) {
        Signal::sigorset(&Scheduler::get_current_thread()->signal_mask, set);
    } else if (how == SIG_UNBLOCK) {
        sigset_t dup = *set;
        Signal::signotset(&dup, &dup);
        Signal::sigandset(&Scheduler::get_current_thread()->signal_mask, &dup);
    } else {
        return -EINVAL;
    }
    return 0;
}

static void sys_sigreturn(ucontext_t* uctx)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_return] %p\n", uctx);
    ThreadContext tctx;
    /*
     * The syscall handler saves the userspace context, so we copy it into
     * tctx to get certain registers (DS, ES, SS) preloaded for us. The
     * rest of the state will get overriden by the stored mcontext
     */
    String::memcpy(&tctx, &Scheduler::get_current_thread()->tcontext,
                   sizeof(tctx));
    // Unset on_stack
    if (Scheduler::get_current_thread()->signal_stack.ss_flags & SS_ONSTACK) {
        Scheduler::get_current_thread()->signal_stack.ss_flags &= ~SS_ONSTACK;
    }
    // Restore signal mask
    Scheduler::get_current_thread()->signal_mask = uctx->uc_sigmask;
    Signal::decode_mcontext(&uctx->uc_mcontext, &tctx);
    load_registers(tctx);
}

static long sys_ioctl(int fd, unsigned long request, char* argp)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_ioctl] = %d, 0x%lX, %p\n", fd,
                request, argp);
    if (!Scheduler::get_current_process()->get_dtable()->get(fd)) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->get_dtable()->get(fd)->ioctl(
        request, argp);
}

static long sys_dup(int oldfd)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_dup] = %d\n", oldfd);
    auto desc = Scheduler::get_current_process()->get_dtable()->get(oldfd);
    if (!desc) {
        return -EBADF;
    }
    return Scheduler::get_current_process()->get_dtable()->add(desc);
}

static long sys_dup2(int oldfd, int newfd)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_dup2] = %d %d\n", oldfd, newfd);
    return Scheduler::get_current_process()->get_dtable()->copy(oldfd, newfd);
}

static long sys_getpid()
{
    // Return process ID not thread ID
    return Scheduler::get_current_process()->pid;
}

static long sys_fork()
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_fork]\n");
    Process* child = Scheduler::get_current_process()->fork();
    Thread* thread = new Thread(child);
    String::memcpy(&thread->tcontext,
                   &Scheduler::get_current_thread()->tcontext,
                   sizeof(thread->tcontext));
    // Child process gets 0 returned from fork
#ifdef X86_64
    thread->tcontext.rax = 0;
#else
    thread->tcontext.eax = 0;
#endif
    thread->tcontext.kernel_stack = (addr_t) new uint8_t[0x1000] + 0x1000;
    Scheduler::insert(thread);
    return child->pid;
}

static long sys_execve(const char* path, const char* old_argv[],
                       const char* old_envp[])
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_execve]: %s %p %p\n", path,
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
        argv[i] = new char[String::strlen(old_argv[i])];
        String::strcpy(const_cast<char*>(argv[i]), old_argv[i]);
    }
    const char** envp = new const char*[envc];
    for (size_t i = 0; i < envc; i++) {
        envp[i] = new char[String::strlen(old_envp[i])];
        String::strcpy(const_cast<char*>(envp[i]), old_envp[i]);
    }
    libcxx::intrusive_ptr<Filesystem::Descriptor> file =
        get_start(path)->open(path, 0, 0);
    if (!file) {
        delete[] envp;
        delete[] argv;
        return -ENOENT;
    }
    struct Filesystem::stat st;
    file->stat(&st);
    Log::printk(Log::LogLevel::DEBUG,
                "[sys_execve] binary has size of %zu bytes\n", st.st_size);
    uint8_t* raw = new uint8_t[st.st_size];
    file->read(raw, st.st_size);
    Scheduler::get_current_process()->sections->reset();
    struct ThreadContext ctx;
    if (!Scheduler::get_current_thread()->load(reinterpret_cast<addr_t>(raw),
                                               argc, argv, envc, envp, ctx)) {
        Log::printk(Log::LogLevel::ERROR, "Failed to load thread state\n");
    }
    delete[] raw;
    load_registers(ctx);
    __builtin_unreachable();
}

static void sys_exit(int val)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_exit] = %d\n", val);
    Scheduler::get_current_thread()->exit();
}

static long sys_kill(pid_t pid, int signum)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_kill] %u %d\n", pid, signum);
    Process* process = Scheduler::find_process(pid);
    if (!process) {
        return -ESRCH;
    }
    Log::printk(Log::LogLevel::DEBUG, "Found process at %p\n", process);
    process->send_signal(signum);
    return 0;
}

static long sys_sigpending(sigset_t* set)
{
    if (!set) {
        return -EFAULT;
    }
    sigset_t pending = Scheduler::get_current_thread()->signal_pending;
    /*
     * Signals that are both blocked and ignored are NOT added to the mask
     * of pending signals.
     *
     * However, signals that are merely blocked will be added
     */
    sigset_t blocked_and_ignored;
    // Get the list of blocked signals
    Signal::sigorset(&blocked_and_ignored,
                     &Scheduler::get_current_thread()->signal_mask);
    // Remove signals that are NOT ignored
    for (int i = 1; i < NSIGS; i++) {
        // TODO: Also check if SIG_DFL is set but the default action is to
        // ignore
        if (Scheduler::get_current_process()->signal_actions[i].sa_handler !=
            SIG_IGN) {
            Signal::sigdelset(&blocked_and_ignored, i);
        }
    }
    // Invert the set so that signals that are both blocked and ignored are
    // unset
    Signal::signotset(&blocked_and_ignored, &blocked_and_ignored);
    // AND the two sets together to basically unset blocked and ignored
    Signal::sigandset(&pending, &blocked_and_ignored);

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
    Log::printk(Log::LogLevel::DEBUG, "[sys_sigaltstack] %p %p\n", ss, oldss);
    if (oldss) {
        String::memcpy(oldss, &Scheduler::get_current_thread()->signal_stack,
                       sizeof(*oldss));
    }
    if (ss) {
        String::memcpy(&Scheduler::get_current_thread()->signal_stack, ss,
                       sizeof(*ss));
    }
    return 0;
}

static long sys_mknod(const char* path, mode_t mode, dev_t dev)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_mknod] %s %X %X\n", path, mode,
                dev);
    return get_start(path)->mknod(path, mode, dev);
}

static long sys_init_module(void* module_image, unsigned long len,
                            const char* param_values)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_init_module] %p %llX %p\n",
                module_image, len, param_values);
    return load_module(module_image);
}

static long sys_delete_module(const char* name, int flags)
{
    Log::printk(Log::LogLevel::DEBUG, "[sys_delete_module] %s %X\n", name,
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
    syscall_table[SYS_kill]        = reinterpret_cast<void*>(sys_kill);
    syscall_table[SYS_sigpending]  = reinterpret_cast<void*>(sys_sigpending);
    syscall_table[SYS_sigaltstack] = reinterpret_cast<void*>(sys_sigaltstack);
    syscall_table[SYS_mknod]       = reinterpret_cast<void*>(sys_mknod);
    syscall_table[SYS_init_module] = reinterpret_cast<void*>(sys_init_module);
    syscall_table[SYS_delete_module] =
        reinterpret_cast<void*>(sys_delete_module);
}
} // namespace Syscall
