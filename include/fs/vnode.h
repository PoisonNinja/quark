#pragma once

#include <types.h>

namespace FS
{
class Vnode
{
public:
    Vnode();
    ~Vnode();

    ino_t ino;
    dev_t dev;
    mode_t mode;

private:
};
}
