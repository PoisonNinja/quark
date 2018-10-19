#include <fs/dtable.h>
#include <kernel.h>

namespace Filesystem
{
DTable::DTable(int s)
{
    fds  = new Ref<Descriptor>[s];
    size = step_size = s;
}

DTable::DTable(const DTable& other)
{
    size      = other.size;
    step_size = other.step_size;
    fds       = new Ref<Descriptor>[other.size];
    for (int i = 0; i < size; i++) {
        fds[i] = other.fds[i];
    }
}

DTable::~DTable()
{
    delete[] fds;
}

void DTable::resize()
{
    Ref<Descriptor>* tmp = new Ref<Descriptor>[size + step_size];
    for (int i = 0; i < size; i++) {
        tmp[i] = fds[i];
    }
    delete[] fds;
    fds = tmp;
    this->size += step_size;
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

Ref<Descriptor> DTable::get(int index)
{
    if (index >= size) {
        return Ref<Descriptor>(nullptr);
    }
    return fds[index];
}

int DTable::copy(int oldfd, int newfd)
{
    this->fds[oldfd] = this->fds[newfd];
    return oldfd;
}
} // namespace Filesystem
