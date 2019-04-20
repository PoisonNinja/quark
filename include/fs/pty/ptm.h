#pragma once

#include <fs/inode.h>
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
    int ioctl(unsigned long command, char* argp) override;
    ssize_t write(const uint8_t* buffer, size_t count) override;

    ssize_t notify(const uint8_t* buffer, size_t count);

    void init_termios(struct termios& termios) override;
    void set_pts(pts* slave);

private:
    pts* slave;
};

class ptmx : public kdevice
{
public:
    ptmx(ptsfs* fs);
    int ioctl(unsigned long request, char* argp, void* cookie) override;
    libcxx::pair<int, void*> open(const char* name) override;
    int poll(filesystem::poll_register_func_t& callback, void* cookie) override;
    ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                 void* cookie) override;
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                  void* cookie) override;

private:
    ptsfs* fs;
};
} // namespace tty
} // namespace filesystem
