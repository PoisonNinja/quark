#pragma once

#include <fs/tty.h>
#include <lib/list.h>
#include <proc/sched.h>

namespace Filesystem
{
namespace TTY
{
constexpr size_t pty_size = 4096;

class PTY
{
public:
    PTY(int index);

    int index();

    ssize_t mread(uint8_t* buffer, size_t count);
    ssize_t mwrite(uint8_t* buffer, size_t count);

    ssize_t sread(uint8_t* buffer, size_t count);
    ssize_t swrite(uint8_t* buffer, size_t count);

    libcxx::node<PTY> node;

private:
    char mbuf[pty_size];
    size_t mhead, mtail;
    Scheduler::WaitQueue mqueue;

    char sbuf[pty_size];
    size_t shead, stail;
    Scheduler::WaitQueue squeue;

    int idx;
};
} // namespace TTY
} // namespace Filesystem
