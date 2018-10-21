#pragma once

#include <fs/descriptor.h>

namespace Filesystem
{
class DTable
{
public:
    DTable(int s = 4);
    DTable(const DTable& other);
    ~DTable();

    int add(std::shared_ptr<Descriptor> desc);
    std::shared_ptr<Descriptor> get(int index);
    int copy(int oldfd, int newfd);
    bool remove(int fd);

private:
    void resize();
    std::shared_ptr<Descriptor>* fds;
    int size;
    int step_size;
};
} // namespace Filesystem
