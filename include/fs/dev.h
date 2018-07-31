#pragma once

#include <types.h>

namespace Filesystem
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

enum DeviceClass { BLK, CHR };

class KDevice
{
public:
    KDevice(DeviceClass type) : type(type){};
    virtual ~KDevice(){};

    const char* name;

    // A subset of Inode operations
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) = 0;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset) = 0;

protected:
    DeviceClass type;
};

dev_t locate_class(DeviceClass c);
bool register_class(DeviceClass c, dev_t major);

bool register_kdevice(DeviceClass c, dev_t major, KDevice* kdev);

KDevice* get_kdevice(mode_t mode, dev_t dev);
}  // namespace Filesystem