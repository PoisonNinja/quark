#pragma once

#include <lib/pair.h>
#include <lib/refcount.h>
#include <types.h>

namespace Filesystem
{
class Inode : public RefcountBase
{
public:
    ino_t ino;
    dev_t rdev;
    mode_t mode;

    virtual ~Inode(){};
    virtual int link(const char* name, Ref<Inode> node)                 = 0;
    virtual Ref<Inode> lookup(const char* name, int flags, mode_t mode) = 0;
    virtual int mkdir(const char* name, mode_t mode)                    = 0;
    virtual int mknod(const char* name, mode_t mode, dev_t dev)         = 0;
    virtual Pair<int, void*> open(const char* name)                     = 0;
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset)   = 0;
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                         void* cookie)                                  = 0;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset)  = 0;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset,
                          void* cookie)                                 = 0;
    virtual int stat(struct stat* st)                                   = 0;
};

class BaseInode : public Inode
{
public:
    BaseInode();
    virtual ~BaseInode();
    virtual int link(const char* name, Ref<Inode> node);
    virtual int mkdir(const char* name, mode_t mode);
    virtual int mknod(const char* name, mode_t mode, dev_t dev);
    virtual Pair<int, void*> open(const char* name);
    virtual Ref<Inode> lookup(const char* name, int flags, mode_t mode);
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset);
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                         void* cookie);
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset);
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset,
                          void* cookie);
    virtual int stat(struct stat* st);

protected:
    size_t size;
    uid_t uid;
    gid_t gid;
};
} // namespace Filesystem
