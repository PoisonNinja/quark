#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>

namespace Filesystem
{
TTY::TTY() : KDevice(CHR)
{
}

TTY::~TTY()
{
}

ssize_t TTY::read(uint8_t * /*buffer*/, size_t /*size*/, off_t /*offset*/)
{
    return -1;
}

ssize_t TTY::write(uint8_t *buffer, size_t size, off_t /*offset*/)
{
    return this->output(buffer, size);
}
}  // namespace Filesystem
