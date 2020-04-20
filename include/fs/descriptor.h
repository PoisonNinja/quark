#pragma once

#include <fs/inode.h>
#include <fs/poll.h>
#include <fs/vnode.h>
#include <kernel/lock.h>
#include <lib/memory.h>

namespace filesystem
{

enum descriptor_flags {
    F_READ  = (1 << 0),
    F_WRITE = (1 << 1),
    // Kernel internal stuff
    F_NOMOUNT = (1 << 31), // Don't follow the last mountpoint
};

constexpr int oflags_to_descriptor(int o_flag)
{
    return o_flag + 1;
}

class descriptor : public libcxx::intrusive_ref_counter
{
public:
    descriptor(libcxx::intrusive_ptr<vnode> vnode, int flags);
    int ioctl(unsigned long request, char* argp);
    int link(const char* name, libcxx::intrusive_ptr<descriptor> node);
    off_t lseek(off_t offset, int whence);
    int mkdir(const char* name, mode_t mode);
    int mknod(const char* name, mode_t mode, dev_t dev);
    int mount(const char* source, const char* target, const char* type,
              unsigned long flags);
    libcxx::pair<int, libcxx::intrusive_ptr<descriptor>>
    open(const char* name, int flags, mode_t mode);
    int poll(poll_register_func_t& callback);
    ssize_t pread(uint8_t* buffer, size_t count, off_t offset);
    ssize_t pwrite(const uint8_t* buffer, size_t count, off_t offset);
    ssize_t read(uint8_t* buffer, size_t count);
    bool seekable();
    int stat(struct stat* st);
    ssize_t write(const uint8_t* buffer, size_t count);

private:
    libcxx::intrusive_ptr<vnode> vno;
    mutex current_offset_mutex;
    off_t current_offset;
    int flags;
};
} // namespace filesystem
