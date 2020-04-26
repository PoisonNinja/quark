#pragma once

#include <fs/inode.h>
#include <fs/tty.h>

namespace filesystem
{
class ptsfs;

namespace terminal
{
class pts;

class ptm : public tty_driver
{
public:
    ptm();
    int ioctl(unsigned long command, char* argp) override;
    ssize_t write(const uint8_t* buffer, size_t count) override;
    void init_termios(struct termios& termios) override;
    void set_pts(tty* slave);

private:
    tty* slave;
};

class ptmx : public kdevice
{
public:
    ptmx(ptsfs* fs);
    int ioctl(unsigned long request, char* argp) override;
    int poll(filesystem::poll_register_func_t& callback) override;
    ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset) override;

private:
    struct tty* tty;
    size_t index;
    ptsfs* fs;
};

class ptmx_mux : public kdevice
{
public:
    ptmx_mux(ptsfs* fs);
    kdevice* factory() override;

private:
    ptsfs* fs;
};
} // namespace terminal
} // namespace filesystem
