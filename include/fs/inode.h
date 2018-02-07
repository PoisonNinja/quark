#pragma once

#include <types.h>

namespace FS
{
class Inode
{
public:
    Inode();
    ~Inode();

    ino_t ino;
    dev_t dev;
    mode_t mode;

private:
};
}
