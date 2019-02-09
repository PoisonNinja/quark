#pragma once

#include <lib/list.h>
#include <types.h>

namespace filesystem
{
#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_APPEND 0x0008
#define O_CREAT 0x0200
#define O_TRUNC 0x0400
#define O_EXCL 0x0800
#define O_NOFOLLOW 0x1000
#define O_PATH 0x2000

// Extensions
#define O_NOMOUNT 0x4000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

void init();
} // namespace filesystem
