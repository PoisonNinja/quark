#pragma once

#include <fs/tty.h>

namespace filesystem
{
class vgafb : public kdevice
{
public:
    vgafb();
    ~vgafb();

    ssize_t write(const uint8_t* buffer, size_t count, off_t offset) override;

private:
    volatile uint16_t* vga_buffer;

    void update_cursor(int col, int row);
};
} // namespace filesystem
