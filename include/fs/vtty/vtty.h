#pragma once

#include <fs/tty.h>

namespace filesystem
{
namespace tty
{
class vtty : public tty_driver
{
public:
    vtty();
    int ioctl(unsigned long request, char* argp) override;
    int open(const char* name) override;
    ssize_t write(const uint8_t* buffer, size_t count) override;
    void init_termios(struct termios& termios) override;

private:
    int x, y;
    int bg, fg;
    uint16_t internal_buffer[80 * 25];
};

void vtty_init();
} // namespace tty
} // namespace filesystem
