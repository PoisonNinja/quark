#pragma once

#include <fs/tty.h>

namespace Filesystem
{
namespace TTY
{
class VGATTY : public KDevice
{
public:
    VGATTY();
    ~VGATTY();

    ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                  void* cookie) override;

private:
    volatile uint16_t* vga_buffer;

    void update_cursor(int col, int row);
};
} // namespace TTY
} // namespace Filesystem
