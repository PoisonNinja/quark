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

    void set_cwd(Ref<Filesystem::Descriptor> desc);
    void set_root(Ref<Filesystem::Descriptor> desc);
    void set_dtable(Ref<Filesystem::DTable> table);

    Ref<Filesystem::Descriptor> get_cwd();
    Ref<Filesystem::Descriptor> get_root();
    Ref<Filesystem::DTable> get_dtable();

    // TLS stuff
    addr_t tls_base;
    addr_t tls_filesz;
    addr_t tls_memsz;
    addr_t tls_alignment;

    void exit();

    Process* fork();

    Memory::SectionManager* sections;

    Node<Process> child_node;

    void send_signal(int signum);
    struct sigaction signal_actions[NSIGS];

    addr_t sigreturn;

private:
    Ref<Filesystem::Descriptor> cwd;
    Ref<Filesystem::Descriptor> root;
    Ref<Filesystem::DTable> fds;

    Process* parent;
    List<Process, &Process::child_node> children;
    List<Thread, &Thread::process_node> threads;
};
