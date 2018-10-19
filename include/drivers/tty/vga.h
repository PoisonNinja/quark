#pragma once

#include <fs/tty.h>

namespace Filesystem
{
namespace TTY
{
class VGATTY : public TTY
{
public:
    VGATTY();
    ~VGATTY();

    ssize_t write(uint8_t* buffer, size_t count);

private:
    volatile uint16_t* vga_buffer;
    int x;
    int y;
    int color;

    static void update_cursor(int col, int row);
};
} // namespace TTY
} // namespace Filesystem
