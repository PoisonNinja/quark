#pragma once

#include <types.h>

namespace Filesystem
{
class Inode
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;
};

class BaseInode : public Inode
{
public:

};
}
