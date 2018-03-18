#pragma once

#include <fs/descriptor.h>

namespace Filesystem
{
class DTable
{
public:
    DTable(int s = 4);
    ~DTable();

    size_t add(Ref<Descriptor> desc);
    bool remove(int fd);
    Ref<Descriptor> operator[](size_t index);

private:
    void resize();
    Ref<Descriptor>* fds;
    size_t size;
    size_t step_size;
};
}
