#pragma once

#include <lib/list.h>
#include <types.h>

namespace Filesystem
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

class Superblock;

class Driver
{
public:
    const char* name;
    virtual int mount(Superblock*);
    Node<Driver> node;
};

status_t register_driver(Driver& driver);
status_t unregister_driver(Driver& driver);
Driver* get_driver(const char* name);

void init();
}
