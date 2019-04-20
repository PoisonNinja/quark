#include <fs/dev.h>
#include <fs/pty/ptm.h>
#include <fs/pty/pts.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>

namespace filesystem
{
namespace tty
{
namespace
{
struct ptmx_metadata {
    struct tty_core* tty;
    size_t index;
};
} // namespace

ptm::ptm()
    : slave(nullptr)
{
}

ssize_t ptm::write(const uint8_t* buffer, size_t count)
{
    return this->slave->notify(buffer, count);
}

void ptm::init_termios(struct ktermios& termios)
{
    termios.c_iflag  = 0;
    termios.c_oflag  = 0;
    termios.c_cflag  = 0;
    termios.c_lflag  = 0;
    termios.c_ispeed = termios.c_ospeed = 38400;
}

ssize_t ptm::notify(const uint8_t* buffer, size_t count)
{
    return this->core->notify(buffer, count);
}

void ptm::set_pts(pts* slave)
{
    this->slave = slave;
}

ptmx::ptmx(ptsfs* fs)
    : kdevice(filesystem::CHR)
    , fs(fs)
{
}

int ptmx::ioctl(unsigned long request, char* argp, void* cookie)
{
    if (request == TIOCGPTN) {
        *reinterpret_cast<int*>(argp) =
            static_cast<ptmx_metadata*>(cookie)->index;
        return 0;
    }
    return static_cast<ptmx_metadata*>(cookie)->tty->ioctl(request, argp,
                                                           cookie);
}

libcxx::pair<int, void*> ptmx::open(const char* name)
{
    ptm* master         = new ptm();
    ptmx_metadata* meta = new ptmx_metadata;
    meta->tty           = register_tty(master, 0, 0, tty_no_register);
    meta->index         = this->fs->register_ptm(master);
    return libcxx::pair<int, void*>(0, meta);
}

int ptmx::poll(filesystem::poll_register_func_t& callback, void* cookie)
{
    return static_cast<ptmx_metadata*>(cookie)->tty->poll(callback, cookie);
}

ssize_t ptmx::read(uint8_t* buffer, size_t count, off_t offset, void* cookie)
{
    return static_cast<ptmx_metadata*>(cookie)->tty->read(buffer, count, offset,
                                                          cookie);
}

ssize_t ptmx::write(const uint8_t* buffer, size_t count, off_t offset,
                    void* cookie)
{
    return static_cast<ptmx_metadata*>(cookie)->tty->write(buffer, count,
                                                           offset, cookie);
}

} // namespace tty
} // namespace filesystem
