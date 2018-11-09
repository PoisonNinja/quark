#pragma once

#include <lib/list.h>
#include <proc/process.h>

class PTableWrapper
{
public:
    PTableWrapper(Process* p)
        : process(p){};
    Process* process;
    libcxx::Node<PTableWrapper> node;
};

class PTable
{
public:
    PTable();
    Process* get(pid_t pid);
    bool add(Process* process);
    bool remove(pid_t pid);

private:
    libcxx::List<PTableWrapper, &PTableWrapper::node> list;
    size_t size;
};