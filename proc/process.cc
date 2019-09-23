#include <arch/cpu/gdt.h>
#include <arch/mm/layout.h>
#include <errno.h>
#include <kernel.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/elf.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/uthread.h>
#include <proc/wait.h>

extern "C" void signal_return();
void* signal_return_location = reinterpret_cast<void*>(&signal_return);

process::process(process* parent)
    : pid(scheduler::get_free_pid())
    , vma(USER_START, USER_END)
    , exit_reason(-1)
    , parent(parent)
{
    for (int i = 1; i < NSIGS; i++) {
        this->signal_actions[i].sa_handler = SIG_DFL;
        this->signal_actions[i].sa_flags   = 0;
    }
}

process::~process()
{
}

pid_t process::get_pid()
{
    return pid;
}

addr_t process::get_address_space() const
{
    return this->address_space;
}

void process::set_address_space(addr_t address)
{
    this->address_space = address;
}

void process::set_cwd(libcxx::intrusive_ptr<filesystem::descriptor> desc)
{
    cwd = desc;
}

void process::set_root(libcxx::intrusive_ptr<filesystem::descriptor> desc)
{
    root = desc;
}

libcxx::intrusive_ptr<filesystem::descriptor> process::get_cwd()
{
    return cwd;
}

libcxx::intrusive_ptr<filesystem::descriptor> process::get_root()
{
    return root;
}

int process::install_desc(libcxx::intrusive_ptr<filesystem::descriptor> file)
{
    return this->fds.add(file);
}

libcxx::intrusive_ptr<filesystem::descriptor> process::get_desc(int fd)
{
    return this->fds.get(fd);
}

int process::copy_desc(int oldfd, int newfd)
{
    return this->fds.copy(oldfd, newfd);
}

bool process::remove_desc(int fd)
{
    return this->fds.remove(fd);
}

void process::set_tls_data(addr_t base, addr_t filesz, addr_t memsz,
                           addr_t alignment)
{
    this->tls_base      = base;
    this->tls_filesz    = filesz;
    this->tls_memsz     = memsz;
    this->tls_alignment = alignment;
}

void process::thread_exit(thread* thread, bool is_signal, int val)
{
    if (!thread) {
        // WTF??
        log::printk(log::log_level::WARNING,
                    "Tried to remove non-existent thread?");
        return;
    }
    threads.erase(threads.iterator_to(*thread));
    if (threads.empty()) {
        log::printk(log::log_level::DEBUG,
                    "Last thread exiting, process %d terminating\n", this->pid);
        this->exit(is_signal, val);
    }
}

process* process::fork()
{
    process* child = new process(this);
    scheduler::add_process(child);
    this->children.push_back(*child);
    addr_t cloned = memory::virt::fork();
    child->fds    = this->fds;
    child->set_root(this->get_root());
    child->set_cwd(this->get_cwd());
    child->vma           = this->vma;
    child->address_space = cloned;
    return child;
}

int process::load(libcxx::intrusive_ptr<filesystem::descriptor> file, int argc,
                  const char* argv[], int envc, const char* envp[],
                  struct thread_context& ctx)
{
    this->vma.reset();
    auto [success, entry] = elf::load(file);
    /*
     * elf::load returns a pair. The first parameter (bool) indicates status,
     * and second parameter is the entry address. If the first parameter is
     * false, the second parameter is undefined (but usually 0)
     */
    if (!success) {
        return -1;
    }
    size_t argv_size = sizeof(char*) * (argc + 1); // argv is null terminated
    size_t envp_size = sizeof(char*) * (envc + 1); // envp is null terminated
    for (int i = 0; i < argc; i++) {
        argv_size += libcxx::strlen(argv[i]) + 1;
    };
    for (int i = 0; i < envc; i++) {
        envp_size += libcxx::strlen(envp[i]) + 1;
    }

    addr_t argv_zone, envp_zone, stack_zone, sigreturn_zone, tls_zone;

    size_t tls_raw_size =
        libcxx::round_up(this->tls_memsz, this->tls_alignment);
    size_t tls_size = tls_raw_size + sizeof(struct uthread);
    tls_size        = libcxx::round_up(tls_size, this->tls_alignment);

    libcxx::tie(success, argv_zone) = this->vma.allocate(USER_START, argv_size);
    if (success) {
        log::printk(log::log_level::DEBUG, "argv located at %p\n", argv_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate argv\n");
        return false;
    }
    libcxx::tie(success, envp_zone) = this->vma.allocate(USER_START, envp_size);
    if (success) {
        log::printk(log::log_level::DEBUG, "envp located at %p\n", envp_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate envp\n");
        return false;
    }
    libcxx::tie(success, stack_zone) =
        this->vma.allocate_reverse(USER_START, 0x1000);
    if (success) {
        log::printk(log::log_level::DEBUG, "Stack located at %p\n", stack_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate stack\n");
        return false;
    }
    libcxx::tie(success, sigreturn_zone) =
        this->vma.allocate(USER_START, 0x1000);
    if (success) {
        log::printk(log::log_level::DEBUG, "Sigreturn page located at %p\n",
                    sigreturn_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate sigreturn page\n");
        return false;
    }
    libcxx::tie(success, tls_zone) = this->vma.allocate(USER_START, tls_size);
    if (success) {
        log::printk(log::log_level::DEBUG, "TLS copy located at %p\n",
                    tls_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate TLS copy\n");
        return false;
    }
    memory::virt::map_range(argv_zone, argv_size,
                            PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    memory::virt::map_range(envp_zone, envp_size,
                            PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    memory::virt::map_range(stack_zone, 0x1000,
                            PAGE_USER | PAGE_NX | PAGE_WRITABLE);
    memory::virt::map_range(sigreturn_zone, 0x1000, PAGE_USER | PAGE_WRITABLE);
    memory::virt::map_range(tls_zone, tls_size,
                            PAGE_USER | PAGE_NX | PAGE_WRITABLE);

    this->sigreturn = sigreturn_zone;
    // Copy in sigreturn trampoline code
    libcxx::memcpy(reinterpret_cast<void*>(sigreturn_zone),
                   signal_return_location, 0x1000);

    // Make it unwritable
    memory::virt::protect(sigreturn_zone, PAGE_USER);

    // Copy TLS data into thread specific data
    libcxx::memcpy(reinterpret_cast<void*>(tls_zone),
                   reinterpret_cast<void*>(this->tls_base), this->tls_filesz);
    libcxx::memset(reinterpret_cast<void*>(tls_zone + this->tls_filesz), 0,
                   this->tls_memsz - this->tls_filesz);

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
    libcxx::memset(reinterpret_cast<void*>(stack_zone), 0, 0x1000);

    // TODO: Move to architecture
    libcxx::memset(&ctx, 0, sizeof(ctx));
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
    ctx.kernel_stack  = cpu::x86_64::TSS::get_stack();
    scheduler::get_current_thread()->tcontext = ctx;
    return 0;
}

addr_t process::get_sigreturn()
{
    return this->sigreturn;
}

void* process::mmap(addr_t addr, size_t length, int prot, int flags,
                    libcxx::intrusive_ptr<filesystem::descriptor> file,
                    off_t offset)
{
    if (flags & MAP_SHARED) {
        log::printk(log::log_level::WARNING,
                    "[mmap] mmap requested "
                    "MMAP_SHARED, you should probably implement"
                    "this\n");
        return MAP_FAILED;
    }
    if (!(flags & MAP_ANONYMOUS)) {
        if (!file) {
            log::printk(log::log_level::WARNING,
                        "[mmap] Non-existent file descriptor");
            return MAP_FAILED;
        }
        // TODO: Verify the file is actually usable
    }
    addr_t placement = addr;
    if (!(flags & MAP_FIXED)) {
        log::printk(log::log_level::DEBUG, "[mmap] Kernel selecting mapping\n");
        auto [ret, value] = this->vma.locate_range(
            (addr) ? reinterpret_cast<addr_t>(addr) : USER_START, length);
        if (!ret) {
            log::printk(log::log_level::WARNING,
                        "[sys_mmap] Failed to allocate area for mmap\n");
            return MAP_FAILED;
        }
        log::printk(log::log_level::DEBUG, "[sys_mmap] Selected %p\n", value);
    } else {
        log::printk(log::log_level::DEBUG,
                    "[mmap] mmap requested with fixed address %p\n", addr);
        if (this->vma.find(addr) || this->vma.find(addr + length)) {
            return MAP_FAILED;
        }
    }
    // Remove old mappings
    this->munmap(addr, length);
    this->vma.add_vmregion(placement, length);
    int real_flags = memory::virt::prot_to_flags(prot);
    memory::virt::map_range(placement, length,
                            real_flags | PAGE_WRITABLE | PAGE_USER);
    if (!(flags & MAP_ANONYMOUS)) {
        file->pread(reinterpret_cast<uint8_t*>(placement), length, offset);
    } else {
        libcxx::memset(reinterpret_cast<void*>(placement), 0, length);
    }
    if (!(real_flags & PAGE_WRITABLE)) {
        memory::virt::protect_range(placement, length, real_flags | PAGE_USER);
    }
    return reinterpret_cast<void*>(placement);
}

int process::munmap(addr_t addr, size_t length)
{
    // Let vma do most of the hard work, but we'll need to unmap ourselves
    auto [ret, zones] = this->vma.free(addr, length);
    if (!ret) {
        for (auto zone : zones) {
            memory::virt::unmap_range(zone.first, zone.second, UNMAP_FREE);
        }
    }
    return ret;
}

void process::exit(bool is_signal, int val)
{
    for (auto& section : vma) {
        memory::virt::unmap_range(section.start(), section.size(), UNMAP_FREE);
    }
    this->vma.reset();
    this->exit_reason = val;
    if (this->parent) {
        this->parent->notify_exit(this);
    }
    // memory::physical::free(this->address_space);
    scheduler::remove_process(this->pid);
}

pid_t process::wait(pid_t pid, int* status, int options)
{
    if (children.empty()) {
        if (status)
            *status = -ECHILD;
        return -1;
    }
    // Verify that the target child exists
    if (pid > 0) {
        bool found = false;
        // Search through children
        for (auto& proc : children) {
            if (proc.pid == pid) {
                found = true;
            }
        }
        // Search through zombies
        for (auto& proc : zombies) {
            if (proc.pid == pid) {
                found = true;
            }
        }
        if (!found) {
            if (status)
                *status = -ECHILD;
            return -1;
        }
    }
    process* zombie = nullptr;
    while (1) {
        for (auto& z : zombies) {
            if (z.pid == pid || pid == -1) {
                zombie = &z;
                goto found;
            }
        }
        if (options & WNOHANG) {
            return 0;
        }
        int ret = this->waiters.wait(scheduler::wait_interruptible);
        if (ret) {
            if (status)
                *status = -EINTR;
            return -1;
        }
    }

found:
    if (status)
        *status = zombie->exit_reason;

    pid_t ret = zombie->pid;

    // TODO: Reap the zombie
    this->zombies.erase(this->zombies.iterator_to(*zombie));
    delete &zombie;

    return ret;
}

thread* process::create_thread()
{
    tid_t tid = this->pid;
    if (!threads.empty()) {
        // Rest of them increment the PID
        tid = scheduler::get_free_pid();
    }
    thread* t = new thread(this, tid);
    this->threads.push_back(*t);
    return t;
}

void process::notify_exit(process* child)
{
    this->waiters.wakeup();
    this->send_signal(SIGCHLD);
    // TODO: Do initial cleanup, such as adding into zombie queue
    this->children.erase(this->children.iterator_to(*child));
    this->zombies.push_front(*child);
}

void process::send_signal(int signum)
{
    // TODO: Broadcast SIGSTOP, SIGCONT, certain signals to all threads
    // TODO: Randomly select a thread instead of the default (or maybe a
    // heuristic?)
    this->threads.front().send_signal(signum);
}
