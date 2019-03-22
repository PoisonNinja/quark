#include <fs/pty/pty.h>
#include <kernel.h>
#include <lib/string.h>

namespace filesystem
{
namespace tty
{
pty::pty(int index)
{
    libcxx::memset(mbuf, 0, pty_size);
    libcxx::memset(sbuf, 0, pty_size);
    this->mhead = this->mtail = this->shead = this->stail = 0;
    this->idx                                             = index;
}

int pty::index()
{
    return this->idx;
}

ssize_t pty::mread(uint8_t* buffer, size_t count)
{
    size_t read = 0;
    while (read < count) {
        if (this->mhead == this->mtail) {
            this->mqueue.wait(scheduler::wait_interruptible);
        }
        buffer[read++] = this->mbuf[this->mtail++ % pty_size];
    }
    return count;
}

ssize_t pty::mwrite(const uint8_t* buffer, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        this->sbuf[this->shead++ % pty_size] = buffer[i];
    }
    this->squeue.wakeup();
    return count;
}

int pty::mpoll(filesystem::poll_register_func_t& callback)
{
    callback(this->mqueue);
    if (this->mhead != this->mtail) {
        return POLLIN;
    }
    return 0;
}

ssize_t pty::sread(uint8_t* buffer, size_t count)
{
    size_t read = 0;
    if (this->shead == this->stail) {
        this->squeue.wait(scheduler::wait_interruptible);
    }
    while (read < count) {
        if (this->shead == this->stail) {
            return read;
        }
        buffer[read++] = this->sbuf[this->stail++ % pty_size];
    }
    return read;
}

ssize_t pty::swrite(const uint8_t* buffer, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        this->mbuf[this->mhead++ % pty_size] = buffer[i];
    }
    this->mqueue.wakeup();
    return count;
}

int pty::spoll(filesystem::poll_register_func_t& callback)
{
    callback(this->squeue);
    if (this->shead != this->stail) {
        return POLLIN;
    }
    return 0;
}

} // namespace tty
} // namespace filesystem
