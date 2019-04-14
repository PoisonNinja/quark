#pragma once

#include <fs/inode.h>
#include <fs/pty/pty.h>
#include <fs/tty.h>

namespace filesystem
{
class ptsfs;

namespace tty
{
class pts;

class ptm : public tty_driver
{
public:
    ptm();
    ssize_t write(const uint8_t* buffer, size_t count);

private:
    pts* slave;
};

class ptmx : public kdevice
{
public:
    ptmx(ptsfs* fs);
    int ioctl(unsigned long request, char* argp, void* cookie) override;
    libcxx::pair<int, void*> open(const char* name) override;
    ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                 void* cookie) override;
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                  void* cookie) override;

private:
    ptsfs* fs;
};
} // namespace tty
} // namespace filesystem
