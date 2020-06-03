#include <arch/cpu/gdt.h>
#include <arch/mm/layout.h>
#include <errno.h>
#include <kernel.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/binfmt/binfmt.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/uthread.h>
#include <proc/wait.h>
#include <proc/work.h>

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

libcxx::intrusive_ptr<filesystem::descriptor>
process::get_start(const char* path)
{
    libcxx::intrusive_ptr<filesystem::descriptor> start(nullptr);
    if (*path == '/') {
        start = this->root;
    } else {
        start = this->cwd;
    }
    return start;
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

int process::load(const char* path,
                  libcxx::intrusive_ptr<filesystem::descriptor> file, int argc,
                  const char* argv[], int envc, const char* envp[],
                  struct thread_context& ctx)
{
    binfmt::binfmt* fmt = binfmt::get(file);

    if (!fmt) {
        return -ENOEXEC;
    }

    this->vma.reset();

    log::printk(log::log_level::INFO,
                "process::load: Using %s to load binary\n", fmt->name());

    struct binfmt::binprm prm = {.path = path,
                                 .file = file,
                                 .argc = argc,
                                 .argv = argv,
                                 .envc = envc,
                                 .envp = envp,
                                 .ctx  = ctx};

    int res = fmt->load(prm);
    return res;
}

addr_t process::get_sigreturn()
{
    return this->sigreturn;
}

void process::set_sigreturn(addr_t addr)
{
    this->sigreturn = addr;
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
        libcxx::optional<addr_t> value;
        if (!(flags & MAP_GROWSDOWN)) {
            value = this->vma.locate_range(
                (addr) ? reinterpret_cast<addr_t>(addr) : USER_START, length);
        } else {
            value = this->vma.locate_range_reverse(
                (addr) ? reinterpret_cast<addr_t>(addr) : USER_END, length);
        }
        if (!value) {
            log::printk(log::log_level::WARNING,
                        "[sys_mmap] Failed to allocate area for mmap\n");
            return MAP_FAILED;
        }
        log::printk(log::log_level::DEBUG, "[sys_mmap] Selected %p\n", *value);
        placement = *value;
    } else {
        log::printk(log::log_level::DEBUG,
                    "[mmap] mmap requested with fixed address %p\n", addr);
        if (this->vma.find(addr) || this->vma.find(addr + length)) {
            return MAP_FAILED;
        }
    }
    // Remove old mappings
    this->munmap(placement, length);
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
    scheduler::work::schedule(libcxx::bind(&process::cleanup, this));
    scheduler::remove_process(this->pid);
}

void process::cleanup()
{
    // Must be run from a worker thread
    assert(scheduler::get_current_process() != this);

    memory::physical::free(this->address_space);
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
        bool ret = this->waiters.wait(scheduler::wait_interruptible,
                                      [&]() { return !zombies.empty(); });
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

int process::sigaction(int signum, struct sigaction* act,
                       struct sigaction* oldact)
{
    if (oldact) {
        libcxx::memcpy(oldact, &this->signal_actions[signum], sizeof(*oldact));
    }
    libcxx::memcpy(&this->signal_actions[signum], act, sizeof(*act));
    return 0;
}

const struct sigaction* process::get_sigaction(int signum)
{
    if (signum >= SIGMIN && signum <= SIGMAX) {
        return &this->signal_actions[signum];
    } else {
        return nullptr;
    }
}

void process::send_signal(int signum)
{
    // TODO: Broadcast SIGSTOP, SIGCONT, certain signals to all threads
    // TODO: Randomly select a thread instead of the default (or maybe a
    // heuristic?)
    this->threads.front().send_signal(signum);
}
