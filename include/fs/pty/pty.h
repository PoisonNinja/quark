#pragma once

#include <fs/tty.h>
#include <lib/list.h>
#include <proc/sched.h>

namespace filesystem
{
namespace tty
{
constexpr size_t pty_size = 4096;

class pty
{
public:
    pty(int index);

    int index();

    ssize_t mread(uint8_t* buffer, size_t count);
    ssize_t mwrite(const uint8_t* buffer, size_t count);

    ssize_t sread(uint8_t* buffer, size_t count);
    ssize_t swrite(const uint8_t* buffer, size_t count);

    libcxx::node<pty> node;

private:
    char mbuf[pty_size];
    size_t mhead, mtail;
    scheduler::wait_queue mqueue;

    char sbuf[pty_size];
    size_t shead, stail;
    scheduler::wait_queue squeue;

    int idx;
};
} // namespace tty
} // namespace filesystem
