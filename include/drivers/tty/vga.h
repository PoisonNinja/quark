#pragma once

#include <fs/tty.h>

namespace Filesystem
{
class VGATTY : public TTY
{
public:
    VGATTY();
    ~VGATTY();

    ssize_t output(uint8_t* buffer, size_t size);

private:
    volatile uint16_t* vga_buffer;
    int x;
    int y;
    int color;

    static void update_cursor(int col, int row);
};
}
