#include <fs/dtable.h>
#include <kernel.h>

namespace Filesystem
{
DTable::DTable()
{
}

DTable::DTable(const DTable& other)
{
    this->fds = other.fds;
}

DTable::~DTable()
{
}

int DTable::add(libcxx::intrusive_ptr<Descriptor> desc)
{
    for (int i = 0; i < fds.size(); i++) {
        if (!fds[i]) {
            fds[i] = desc;
            return i;
        }
    }

    fds.push_back(desc);
    return fds.size() - 1;
}

bool DTable::remove(int fd)
{
    if (fd >= fds.size() || fd < 0 || !fds[fd]) {
        return false;
    } else {
        fds[fd] = libcxx::intrusive_ptr<Filesystem::Descriptor>(nullptr);
        return true;
    }
}

libcxx::intrusive_ptr<Descriptor> DTable::get(int index)
{
    if (index >= fds.size() || index < 0) {
        return libcxx::intrusive_ptr<Descriptor>(nullptr);
    }
    return fds[index];
}

int DTable::copy(int oldfd, int newfd)
{
    this->fds[oldfd] = this->fds[newfd];
    return oldfd;
}
} // namespace Filesystem
