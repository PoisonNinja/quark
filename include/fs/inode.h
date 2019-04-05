#pragma once

#include <fs/poll.h>
#include <lib/memory.h>
#include <lib/utility.h>
#include <types.h>

namespace filesystem
{
class inode : public libcxx::intrusive_ref_counter
{
public:
    ino_t ino;
    dev_t rdev;
    mode_t mode;

    size_t size;
    uid_t uid;
    gid_t gid;

    inode();
    virtual ~inode();
    virtual int ioctl(unsigned long request, char* argp);
    virtual int link(const char* name, libcxx::intrusive_ptr<inode> node);
    virtual libcxx::intrusive_ptr<inode> lookup(const char* name, int flags,
                                                mode_t mode);
    virtual int mkdir(const char* name, mode_t mode);
    virtual int mknod(const char* name, mode_t mode, dev_t dev);
    virtual libcxx::pair<int, void*> open(const char* name);
    virtual int poll(poll_register_func_t& callback);
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset);
    virtual ssize_t write(const uint8_t* buffer, size_t count, off_t offset);
    virtual int stat(struct stat* st);

    virtual bool seekable();
};
} // namespace filesystem
