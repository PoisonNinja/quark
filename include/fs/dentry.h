#pragma once

#include <types.h>

namespace Filesystem
{
const size_t DENTRY_MAX_LENGTH = 256;

class Superblock;

struct Dentry {
    ino_t ino;
    Superblock* superblock;
    char name[DENTRY_MAX_LENGTH];
};
}
