#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>

namespace Filesystem
{
TTY::TTY()
{
    this->mode |= S_IFCHR;
}

TTY::~TTY()
{
}

ssize_t TTY::read(uint8_t * /*buffer*/, size_t /*size*/)
{
    return -1;
}

ssize_t TTY::write(uint8_t *buffer, size_t size)
{
    return this->output(buffer, size);
}
}