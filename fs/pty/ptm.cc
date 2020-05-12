#include <errno.h>
#include <fs/dev.h>
#include <fs/ioctl.h>
#include <fs/pty/ptm.h>
#include <fs/pty/pts.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>

namespace filesystem
{
namespace terminal
{
ptm::ptm()
    : slave(nullptr)
{
}

int ptm::ioctl(unsigned long command, char* argp)
{
    switch (command) {
        case TIOCSWINSZ:
            this->slave->winch(reinterpret_cast<struct winsize*>(argp));
            return 0;
    }
    return -EINVAL;
}

ssize_t ptm::write(const uint8_t* buffer, size_t count)
{
    return this->slave->handle_input(buffer, count);
}

void ptm::init_termios(struct termios& termios)
{
    termios.c_iflag  = 0;
    termios.c_oflag  = 0;
    termios.c_cflag  = 0;
    termios.c_lflag  = 0;
    termios.c_ispeed = termios.c_ospeed = 38400;
}

void ptm::set_pts(tty* slave)
{
    this->slave = slave;
}

ptmx::ptmx(ptsfs* fs)
    : kdevice(filesystem::CHR)
    , fs(fs)
{
    ptm* master   = new ptm();
    this->tty     = register_tty(master, 0, 0);
    dev_t pts_dev = this->fs->register_ptm(this->tty);
    this->index   = minor(pts_dev);
    master->set_pts(get_tty(major(pts_dev), minor(pts_dev)));
}

int ptmx::ioctl(unsigned long request, char* argp)
{
    if (request == TIOCGPTN) {
        *reinterpret_cast<int*>(argp) = this->index;
        return 0;
    }
    return this->tty->ioctl(request, argp);
}

int ptmx::poll(filesystem::poll_register_func_t& callback)
{
    return this->tty->poll(callback);
}

ssize_t ptmx::read(uint8_t* buffer, size_t count, off_t offset)
{
    return this->tty->read(buffer, count, offset);
}

ssize_t ptmx::write(const uint8_t* buffer, size_t count, off_t offset)
{
    return this->tty->write(buffer, count, offset);
}

ptmx_mux::ptmx_mux(ptsfs* fs)
    : kdevice(filesystem::CHR)
    , fs(fs)
{
}

kdevice* ptmx_mux::factory()
{
    return new ptmx(fs);
}

} // namespace terminal
} // namespace filesystem
