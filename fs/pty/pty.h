#pragma once

#include <fs/tty.h>

namespace Filesystem
{
namespace TTY
{
class PTY
{
public:
    PTY();

    ssize_t mread(uint8_t* buffer, size_t count);
    ssize_t mwrite(uint8_t* buffer, size_t count);

    ssize_t sread(uint8_t* buffer, size_t count);
    ssize_t swrite(uint8_t* buffer, size_t count);

private:
    char mbuf[1024];
    char sbuf[1024];
};
} // namespace TTY
} // namespace Filesystem
