#include <fs/pty/pty.h>
#include <kernel.h>
#include <lib/string.h>

namespace Filesystem
{
namespace TTY
{
PTY::PTY(int index)
{
    this->idx = index;
}

int PTY::index()
{
    return this->idx;
}

ssize_t PTY::mread(uint8_t* buffer, size_t count)
{
    String::memcpy(buffer, mbuf, count);
    return count;
}

ssize_t PTY::mwrite(uint8_t* buffer, size_t count)
{
    String::memcpy(sbuf, buffer, count);
    return count;
}

ssize_t PTY::sread(uint8_t* buffer, size_t count)
{
    String::memcpy(buffer, sbuf, count);
    return count;
}

ssize_t PTY::swrite(uint8_t* buffer, size_t count)
{
    String::memcpy(mbuf, buffer, count);
    return count;
}

} // namespace TTY
} // namespace Filesystem