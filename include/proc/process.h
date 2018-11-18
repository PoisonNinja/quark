#pragma once

#include <fs/descriptor.h>
#include <fs/dtable.h>
#include <lib/list.h>
#include <proc/thread.h>
#include <types.h>

class Process
{
public:
    Process(Process* p);
    ~Process();
    pid_t pid;
    addr_t address_space;
    void add_thread(Thread* thread);
    void remove_thread(Thread* thread);

    void set_cwd(libcxx::intrusive_ptr<Filesystem::Descriptor> desc);
    void set_root(libcxx::intrusive_ptr<Filesystem::Descriptor> desc);

    libcxx::intrusive_ptr<Filesystem::Descriptor> get_cwd();
    libcxx::intrusive_ptr<Filesystem::Descriptor> get_root();
    Filesystem::DTable fds;

    // TLS stuff
    addr_t tls_base;
    addr_t tls_filesz;
    addr_t tls_memsz;
    addr_t tls_alignment;

    void exit();

    Process* fork();

    Memory::SectionManager* sections;

    libcxx::node<Process> child_node;

    void send_signal(int signum);
    struct sigaction signal_actions[NSIGS];

    addr_t sigreturn;

private:
    libcxx::intrusive_ptr<Filesystem::Descriptor> cwd;
    libcxx::intrusive_ptr<Filesystem::Descriptor> root;

    Process* parent;
    libcxx::list<Process, &Process::child_node> children;
    libcxx::list<Thread, &Thread::process_node> threads;
};
