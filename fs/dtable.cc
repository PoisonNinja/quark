#include <fs/dtable.h>
#include <kernel.h>

namespace Filesystem
{
DTable::DTable(int s)
{
    fds  = new std::shared_ptr<Descriptor>[s];
    size = step_size = s;
}

DTable::DTable(const DTable& other)
{
    size      = other.size;
    step_size = other.step_size;
    fds       = new std::shared_ptr<Descriptor>[other.size];
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
    std::shared_ptr<Descriptor>* tmp =
        new std::shared_ptr<Descriptor>[size + step_size];
    for (int i = 0; i < size; i++) {
        tmp[i] = fds[i];
    }
    delete[] fds;
    fds = tmp;
    this->size += step_size;
}

int DTable::add(std::shared_ptr<Descriptor> desc)
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
        fds[fd] = std::shared_ptr<Filesystem::Descriptor>(nullptr);
        return true;
    }
}

std::shared_ptr<Descriptor> DTable::get(int index)
{
    if (index >= size) {
        return std::shared_ptr<Descriptor>(nullptr);
    }
    return fds[index];
}

int DTable::copy(int oldfd, int newfd)
{
    this->fds[oldfd] = this->fds[newfd];
    return oldfd;
}
} // namespace Filesystem
