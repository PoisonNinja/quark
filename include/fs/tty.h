#pragma once

#include <fs/dev.h>
#include <fs/inode.h>

namespace filesystem
{
namespace tty
{
class tty
{
public:
    tty();
    ~tty();

    virtual libcxx::pair<int, void*> open(const char* name);
    virtual int ioctl(unsigned long request, char* argp, void* cookie);

    virtual ssize_t read(uint8_t* buffer, size_t count, void* cookie);
    virtual ssize_t write(const uint8_t* buffer, size_t count, void* cookie);

private:
};

bool register_tty(dev_t major, tty* tty);

void init();
} // namespace tty
} // namespace filesystem
