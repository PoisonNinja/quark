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
    for (unsigned i = 0; i < fds.size(); i++) {
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
    /*
     * Since we check if fd < 0 first thus short circuiting if it's true
     * casting fd to unsigned guarantees that we'll get a valid size
     */
    if (fd < 0 || static_cast<unsigned>(fd) >= fds.size() || !fds[fd]) {
        return false;
    } else {
        fds[fd] = libcxx::intrusive_ptr<Filesystem::Descriptor>(nullptr);
        return true;
    }
}

libcxx::intrusive_ptr<Descriptor> DTable::get(int index)
{
    if (index < 0 || static_cast<unsigned>(index) >= fds.size()) {
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
