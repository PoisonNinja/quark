#pragma once

#include <types.h>

namespace Filesystem
{
class Inode;
namespace Path
{
Inode* resolve(const char* path, int flags, mode_t mode);
}
}
