#include <fs/dtable.h>

namespace Filesystem
{
DTable::DTable(int s)
{
    fds = new Ref<Descriptor>[s];
    size = step_size = s;
}

DTable::~DTable()
{
    delete[] fds;
}

DTable& DTable::operator=(const DTable& d)
{
    size = d.size;
    step_size = d.step_size;
    fds = new Ref<Descriptor>[d.size];
    for (int i = 0; i < size; i++) {
        fds[i] = d.fds[i];
    }
    return *this;
}

void DTable::resize()
{
    Ref<Descriptor>* tmp = new Ref<Descriptor>[size + step_size];
    for (int i = 0; i < size; i++) {
        tmp[i] = fds[i];
    }
    delete[] fds;
    fds = tmp;
}

int DTable::add(Ref<Descriptor> desc)
{
    for (int i = 0; i < size; i++) {
        if (!fds[i]) {
            fds[i] = desc;
            return i;
        }
    }
    resize();
    fds[size - step_size] = desc;
    return size - step_size;
}

bool DTable::remove(int fd)
{
    if (fd >= size || !fds[fd]) {
        return false;
    } else {
        fds[fd] = Ref<Filesystem::Descriptor>(nullptr);
        return true;
    }
}

Ref<Descriptor> DTable::operator[](int index)
{
    if (index >= size) {
        return Ref<Descriptor>(nullptr);
    }
    return fds[index];
}
}
