#pragma once

#include <fs/poll.h>
#include <lib/utility.h>
#include <types.h>

namespace filesystem
{
constexpr dev_t major(dev_t dev)
{
    return (dev >> 8) & 0xFF;
}

constexpr dev_t minor(dev_t dev)
{
    return (dev >> 0) & 0xFF;
}

constexpr dev_t mkdev(dev_t major, dev_t minor)
{
    return ((major & 0xFF) << 8) | ((minor & 0xFF) << 0);
}

enum device_class { BLK, CHR };

class kdevice
{
public:
    kdevice(device_class type)
        : type(type){};
    virtual ~kdevice(){};

    const char* name;

    // A subset of Inode operations
    virtual libcxx::pair<int, void*> open(const char* name);
    virtual int ioctl(unsigned long request, char* argp);
    virtual int poll(poll_register_func_t& callback);
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset);
    virtual ssize_t write(const uint8_t* buffer, size_t count, off_t offset);

    virtual bool seekable();

protected:
    device_class type;
};

dev_t locate_class(device_class c);
bool register_class(device_class c, dev_t major);

bool register_kdevice(device_class c, dev_t major, kdevice* kdev);

kdevice* get_kdevice(mode_t mode, dev_t dev);
} // namespace filesystem
