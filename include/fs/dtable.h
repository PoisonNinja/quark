#pragma once

#include <fs/descriptor.h>
#include <kernel/lock.h>
#include <lib/memory.h>
#include <lib/vector.h>

namespace filesystem
{
class dtable
{
public:
    dtable();
    dtable(const dtable& other);
    ~dtable();

    dtable& operator=(const dtable& other);

    int add(libcxx::intrusive_ptr<descriptor> desc);
    libcxx::intrusive_ptr<descriptor> get(int index);
    int copy(int oldfd, int newfd);
    bool remove(int fd);

private:
    libcxx::vector<libcxx::intrusive_ptr<descriptor>> fds;
    mutex fds_mutex;
};
} // namespace filesystem
