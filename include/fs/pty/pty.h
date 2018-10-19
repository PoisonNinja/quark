#pragma once

#include <fs/tty.h>
#include <lib/list.h>

namespace Filesystem
{
namespace TTY
{
class PTY
{
public:
    PTY(int index);

    int index();

    ssize_t mread(uint8_t* buffer, size_t count);
    ssize_t mwrite(uint8_t* buffer, size_t count);

    ssize_t sread(uint8_t* buffer, size_t count);
    ssize_t swrite(uint8_t* buffer, size_t count);

    Node<PTY> node;

private:
    char mbuf[1024];
    char sbuf[1024];

    int idx;
};
} // namespace TTY
} // namespace Filesystem
