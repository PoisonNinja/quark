#pragma once

#include <fs/descriptor.h>
#include <lib/memory.h>
#include <lib/vector.h>

namespace filesystem
{
class DTable
{
public:
    DTable();
    DTable(const DTable& other);
    ~DTable();

    int add(libcxx::intrusive_ptr<Descriptor> desc);
    libcxx::intrusive_ptr<Descriptor> get(int index);
    int copy(int oldfd, int newfd);
    bool remove(int fd);

private:
    libcxx::vector<libcxx::intrusive_ptr<Descriptor>> fds;
};
} // namespace filesystem
