#pragma once

#include <fs/descriptor.h>
#include <fs/dtable.h>
#include <lib/list.h>
#include <mm/vma.h>
#include <proc/thread.h>
#include <proc/wq.h>
#include <types.h>

class process
{
public:
    process(process* p);
    ~process();

    pid_t get_pid();

    addr_t get_address_space() const;
    void set_address_space(addr_t address);

    void thread_exit(thread* thread, bool is_signal, int val);

    void set_cwd(libcxx::intrusive_ptr<filesystem::descriptor> desc);
    void set_root(libcxx::intrusive_ptr<filesystem::descriptor> desc);

    libcxx::intrusive_ptr<filesystem::descriptor> get_cwd();
    libcxx::intrusive_ptr<filesystem::descriptor> get_root();

    int install_desc(libcxx::intrusive_ptr<filesystem::descriptor> file);
    libcxx::intrusive_ptr<filesystem::descriptor> get_desc(int fd);
    int copy_desc(int oldfd, int newfd);
    bool remove_desc(int fd);

    addr_t get_sigreturn();
    void set_sigreturn(addr_t);

    // Memory ops
    void* mmap(addr_t addr, size_t length, int prot, int flags,
               libcxx::intrusive_ptr<filesystem::descriptor> file,
               off_t offset);
    int munmap(addr_t addr, size_t length);

    void set_tls_data(addr_t base, addr_t filesz, addr_t memsz,
                      addr_t alignment);

    void exit(bool is_signal, int val);
    void cleanup();

    pid_t wait(pid_t pid, int* status, int options);
    void notify_exit(process* child);

    process* fork();
    int load(libcxx::intrusive_ptr<filesystem::descriptor> file, int argc,
             const char* argv[], int envc, const char* envp[],
             struct thread_context& ctx);
    libcxx::intrusive_ptr<filesystem::descriptor> get_start(const char* path);

    thread* create_thread();

    int sigaction(int signum, struct sigaction* act, struct sigaction* oldact);
    const struct sigaction* get_sigaction(int index);
    void send_signal(int signum);

private:
    libcxx::node<process> list_node;
    friend class libcxx::list<process, &process::list_node>;

    pid_t pid;

    filesystem::dtable fds;

    addr_t address_space;
    addr_t sigreturn;

    // TLS stuff
    addr_t tls_base;
    addr_t tls_filesz;
    addr_t tls_memsz;
    addr_t tls_alignment;

    memory::vma vma;

    struct sigaction signal_actions[NSIGS];

    // waitpid stuff
    scheduler::wait_queue waiters;

    int exit_reason;

    libcxx::intrusive_ptr<filesystem::descriptor> cwd;
    libcxx::intrusive_ptr<filesystem::descriptor> root;

    process* parent;
    libcxx::list<process, &process::list_node> zombies;
    libcxx::list<process, &process::list_node> children;
    libcxx::list<thread, &thread::process_node> threads;
};
