#pragma once

#include <fs/dev.h>
#include <fs/inode.h>

namespace Filesystem
{
class TTY : public KDevice
{
public:
    TTY();
    ~TTY();

    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset) override;

    virtual ssize_t output(uint8_t* buffer, size_t count) = 0;

private:
};
}  // namespace Filesystem
