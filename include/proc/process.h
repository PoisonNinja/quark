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

    pid_t pid;
    addr_t address_space;

    void add_thread(thread* thread);
    void thread_exit(thread* thread, bool is_signal, int val);

    void set_cwd(libcxx::intrusive_ptr<filesystem::descriptor> desc);
    void set_root(libcxx::intrusive_ptr<filesystem::descriptor> desc);

    libcxx::intrusive_ptr<filesystem::descriptor> get_cwd();
    libcxx::intrusive_ptr<filesystem::descriptor> get_root();
    filesystem::dtable fds;

    // TLS stuff
    addr_t tls_base;
    addr_t tls_filesz;
    addr_t tls_memsz;
    addr_t tls_alignment;

    void exit(bool is_signal, int val);

    pid_t wait(pid_t pid, int* status, int options);
    void notify_exit(process* child);

    process* fork();
    int load(addr_t binary, int argc, const char* argv[], int envc,
             const char* envp[], struct thread_context& ctx);

    memory::vma* vma;

    libcxx::node<process> list_node;

    void send_signal(int signum);
    struct sigaction signal_actions[NSIGS];

    addr_t sigreturn;

private:
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
