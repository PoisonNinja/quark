#pragma once

#include <fs/descriptor.h>
#include <lib/memory.h>

namespace Filesystem
{
class DTable : public libcxx::intrusive_ref_counter
{
public:
    DTable(int s = 4);
    DTable(const DTable& other);
    ~DTable();

    int add(libcxx::intrusive_ptr<Descriptor> desc);
    libcxx::intrusive_ptr<Descriptor> get(int index);
    int copy(int oldfd, int newfd);
    bool remove(int fd);

private:
    void resize();
    libcxx::intrusive_ptr<Descriptor>* fds;
    int size;
    int step_size;
};
} // namespace Filesystem
