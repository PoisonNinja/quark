#pragma once

#include <lib/pair.h>
#include <memory>
#include <types.h>

namespace Filesystem
{
class Inode
{
public:
    ino_t ino;
    dev_t rdev;
    mode_t mode;

    virtual ~Inode(){};
    virtual int ioctl(unsigned long request, char* argp, void* cookie) = 0;
    virtual int link(const char* name, std::shared_ptr<Inode> node)    = 0;
    virtual std::shared_ptr<Inode> lookup(const char* name, int flags,
                                          mode_t mode)                 = 0;
    virtual int mkdir(const char* name, mode_t mode)                   = 0;
    virtual int mknod(const char* name, mode_t mode, dev_t dev)        = 0;
    virtual Pair<int, void*> open(const char* name)                    = 0;
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                         void* cookie)                                 = 0;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset,
                          void* cookie)                                = 0;
    virtual int stat(struct stat* st)                                  = 0;

    virtual bool seekable() = 0;
};

class BaseInode : public Inode
{
public:
    BaseInode();
    virtual ~BaseInode();
    virtual int ioctl(unsigned long request, char* argp, void* cookie);
    virtual int link(const char* name, std::shared_ptr<Inode> node);
    virtual int mkdir(const char* name, mode_t mode);
    virtual int mknod(const char* name, mode_t mode, dev_t dev);
    virtual Pair<int, void*> open(const char* name);
    virtual std::shared_ptr<Inode> lookup(const char* name, int flags,
                                          mode_t mode);
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                         void* cookie);
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset,
                          void* cookie);
    virtual int stat(struct stat* st);

    virtual bool seekable();

protected:
    size_t size;
    uid_t uid;
    gid_t gid;
};
} // namespace Filesystem
