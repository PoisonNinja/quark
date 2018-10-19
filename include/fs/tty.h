#pragma once

#include <fs/dev.h>
#include <fs/inode.h>

namespace Filesystem
{
namespace TTY
{
class TTY
{
public:
    TTY();
    ~TTY();

    virtual Pair<int, void*> open(const char* name);
    virtual int ioctl(unsigned long request, char* argp);

    virtual ssize_t read(uint8_t* buffer, size_t count);
    virtual ssize_t read(uint8_t* buffer, size_t count, void* cookie);
    virtual ssize_t write(uint8_t* buffer, size_t count);
    virtual ssize_t write(uint8_t* buffer, size_t count, void* cookie);

private:
};

bool register_tty(dev_t major, TTY* tty);

void init();
} // namespace TTY
} // namespace Filesystem
