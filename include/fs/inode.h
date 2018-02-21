#pragma once

#include <lib/refcount.h>
#include <types.h>

namespace Filesystem
{
class Inode : public RefcountBase
{
public:
    ino_t ino;
    dev_t dev;
    mode_t mode;

    virtual ~Inode(){};
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode) = 0;
    virtual ssize_t pread(uint8_t* buffer, size_t count, off_t offset) = 0;
    virtual ssize_t pwrite(uint8_t* buffer, size_t count, off_t offset) = 0;
    virtual ssize_t read(uint8_t* buffer, size_t count) = 0;
    virtual ssize_t write(uint8_t* buffer, size_t count) = 0;
};

class BaseInode : public Inode
{
public:
    BaseInode();
    virtual ~BaseInode();
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode);
    virtual ssize_t pread(uint8_t* buffer, size_t count, off_t offset);
    virtual ssize_t pwrite(uint8_t* buffer, size_t count, off_t offset);
    virtual ssize_t read(uint8_t* buffer, size_t count);
    virtual ssize_t write(uint8_t* buffer, size_t count);
};
}
