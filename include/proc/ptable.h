#pragma once

#include <lib/list.h>
#include <proc/process.h>

class ptable_wrapper
{
public:
    ptable_wrapper(process* p)
        : p(p){};
    process* p;
    libcxx::node<ptable_wrapper> node;
};

class ptable
{
public:
    ptable();
    process* get(pid_t pid);
    bool add(process* process);
    bool remove(pid_t pid);

private:
    libcxx::list<ptable_wrapper, &ptable_wrapper::node> list;
    size_t size;
};
