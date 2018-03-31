#pragma once

#include <fs/descriptor.h>

namespace Filesystem
{
class DTable
{
public:
    DTable(int s = 4);
    ~DTable();

    int add(Ref<Descriptor> desc);
    bool remove(int fd);
    Ref<Descriptor> operator[](int index);

private:
    void resize();
    Ref<Descriptor>* fds;
    int size;
    int step_size;
};
}
