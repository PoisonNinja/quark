#pragma once

#include <types.h>

namespace FS
{
const size_t DENTRY_MAX_LENGTH = 256;

struct Dentry {
    ino_t ino;
    char name[DENTRY_MAX_LENGTH];
};
}
