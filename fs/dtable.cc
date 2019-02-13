#include <fs/dtable.h>
#include <kernel.h>

namespace filesystem
{
dtable::dtable()
{
}

dtable::dtable(const dtable& other)
{
    this->fds = other.fds;
}

dtable::~dtable()
{
}

dtable& dtable::operator=(const dtable& other)
{
    this->fds = other.fds;
    return *this;
}

int dtable::add(libcxx::intrusive_ptr<descriptor> desc)
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

bool dtable::remove(int fd)
{
    /*
     * Since we check if fd < 0 first thus short circuiting if it's true
     * casting fd to unsigned guarantees that we'll get a valid size
     */
    if (fd < 0 || static_cast<unsigned>(fd) >= fds.size() || !fds[fd]) {
        return false;
    } else {
        fds[fd] = libcxx::intrusive_ptr<filesystem::descriptor>(nullptr);
        return true;
    }
}

libcxx::intrusive_ptr<descriptor> dtable::get(int index)
{
    if (index < 0 || static_cast<unsigned>(index) >= fds.size()) {
        return libcxx::intrusive_ptr<descriptor>(nullptr);
    }
    return fds[index];
}

int dtable::copy(int oldfd, int newfd)
{
    this->fds[oldfd] = this->fds[newfd];
    return oldfd;
}
} // namespace filesystem
