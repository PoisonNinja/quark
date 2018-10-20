#pragma once

#include <fs/tty.h>
#include <lib/list.h>
#include <proc/sched.h>

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
    size_t mhead, mtail;
    Scheduler::WaitQueue mqueue;

    char sbuf[1024];
    size_t shead, stail;
    Scheduler::WaitQueue squeue;

    int idx;
};
} // namespace TTY
} // namespace Filesystem
