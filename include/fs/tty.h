#pragma once

#include <fs/inode.h>

namespace Filesystem
{
class TTY : public BaseInode
{
public:
    TTY();
    ~TTY();

    virtual ssize_t read(uint8_t* buffer, size_t count) override;
    virtual ssize_t write(uint8_t* buffer, size_t count) override;

    virtual ssize_t output(uint8_t* buffer, size_t count) = 0;

private:
};
}