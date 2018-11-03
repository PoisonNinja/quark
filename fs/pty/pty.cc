#include <fs/pty/pty.h>
#include <kernel.h>
#include <lib/string.h>

namespace Filesystem
{
namespace TTY
{
PTY::PTY(int index)
{
    String::memset(mbuf, 0, pty_size);
    String::memset(sbuf, 0, pty_size);
    this->mhead = this->mtail = this->shead = this->stail = 0;
    this->idx                                             = index;
}

int PTY::index()
{
    return this->idx;
}

ssize_t PTY::mread(uint8_t* buffer, size_t count)
{
    size_t read = 0;
    while (read < count) {
        if (this->mhead == this->mtail) {
            this->mqueue.wait(Scheduler::wait_interruptible);
        }
        buffer[read++] = this->mbuf[this->mtail++ % pty_size];
    }
    return count;
}

ssize_t PTY::mwrite(uint8_t* buffer, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        this->sbuf[this->shead++ % pty_size] = buffer[i];
    }
    this->squeue.wakeup();
    return count;
}

ssize_t PTY::sread(uint8_t* buffer, size_t count)
{
    size_t read = 0;
    while (read < count) {
        if (this->shead == this->stail) {
            this->squeue.wait(Scheduler::wait_interruptible);
        }
        buffer[read++] = this->sbuf[this->stail++ % pty_size];
    }
    return count;
}

ssize_t PTY::swrite(uint8_t* buffer, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        this->mbuf[this->mhead++ % pty_size] = buffer[i];
    }
    this->mqueue.wakeup();
    return count;
}

} // namespace TTY
} // namespace Filesystem
