#pragma once

#include <fs/descriptor.h>

namespace Filesystem
{
class DTable : public RefcountBase
{
public:
    DTable(int s = 4);
    DTable(const DTable& other);
    ~DTable();

    int add(Ref<Descriptor> desc);
    Ref<Descriptor> get(int index);
    int copy(int oldfd, int newfd);
    bool remove(int fd);

private:
    void resize();
    Ref<Descriptor>* fds;
    int size;
    int step_size;
};
} // namespace Filesystem
