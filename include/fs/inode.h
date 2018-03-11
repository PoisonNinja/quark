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
    virtual int link(const char* name, Ref<Inode> node) = 0;
    virtual int mkdir(const char* name, mode_t mode) = 0;
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode) = 0;
    virtual ssize_t pread(uint8_t* buffer, size_t count, off_t offset) = 0;
    virtual ssize_t pwrite(uint8_t* buffer, size_t count, off_t offset) = 0;
    virtual ssize_t read(uint8_t* buffer, size_t count) = 0;
    virtual int stat(struct stat* st) = 0;
    virtual ssize_t write(uint8_t* buffer, size_t count) = 0;
};

class BaseInode : public Inode
{
public:
    BaseInode();
    virtual ~BaseInode();
    virtual int link(const char* name, Ref<Inode> node);
    virtual int mkdir(const char* name, mode_t mode);
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode);
    virtual ssize_t pread(uint8_t* buffer, size_t count, off_t offset);
    virtual ssize_t pwrite(uint8_t* buffer, size_t count, off_t offset);
    virtual ssize_t read(uint8_t* buffer, size_t count);
    virtual int stat(struct stat* st);
    virtual ssize_t write(uint8_t* buffer, size_t count);

protected:
    size_t size;
    uid_t uid;
    gid_t gid;
};
}
