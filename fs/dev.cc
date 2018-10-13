#include <errno.h>
#include <fs/dev.h>
#include <fs/stat.h>
#include <kernel.h>

namespace Filesystem
{
namespace
{
constexpr size_t max_major = 256;
constexpr size_t max_minor = 256;

struct KDeviceClass {
    KDevice* minors[max_minor];
    KDevice* get_kdevice(int minor);
    void add_kdevice(KDevice* kdevice);
};

void KDeviceClass::add_kdevice(KDevice* kdevice)
{
    for (size_t i = 0; i < max_minor; i++) {
        if (!minors[i]) {
            minors[i] = kdevice;
            return;
        }
    }
}

KDevice* KDeviceClass::get_kdevice(int minor)
{
    return minors[minor];
}

KDeviceClass* chrdev[max_major] = {nullptr};
KDeviceClass* blkdev[max_major] = {nullptr};
} // namespace

dev_t locate_class(DeviceClass c)
{
    switch (c) {
        case BLK:
            for (dev_t i = 0; i < max_major; i++) {
                if (!blkdev[i]) {
                    return i;
                }
            }
            break;
        case CHR:
            for (dev_t i = 0; i < max_major; i++) {
                if (!chrdev[i]) {
                    return i;
                }
            }
            break;
    }
    return -1;
}

bool register_class(DeviceClass c, dev_t major)
{
    switch (c) {
        case BLK:
            if (blkdev[major]) {
                Log::printk(Log::LogLevel::WARNING,
                            "Block device already exists with major %llX\n",
                            major);
                return false;
            }
            blkdev[major] = new KDeviceClass();
            return true;
        case CHR:
            if (chrdev[major]) {
                Log::printk(Log::LogLevel::WARNING,
                            "Character device already exists with major %llX\n",
                            major);
                return false;
            }
            chrdev[major] = new KDeviceClass();
            return true;
        default:
            Log::printk(Log::LogLevel::ERROR,
                        "Somehow got a invalid class while "
                        "registering a kernel device\n");
            return false;
    }
}

bool register_kdevice(DeviceClass c, dev_t major, KDevice* kdev)
{
    switch (c) {
        case BLK:
            if (!blkdev[major]) {
                Log::printk(Log::LogLevel::WARNING,
                            "Block device with major %llX not found\n", major);
                return false;
            }
            blkdev[major]->add_kdevice(kdev);
            return true;
        case CHR:
            if (!chrdev[major]) {
                Log::printk(Log::LogLevel::WARNING,
                            "Character device with major %llX not found\n",
                            major);
                return false;
            }
            chrdev[major]->add_kdevice(kdev);
            return true;
        default:
            Log::printk(Log::LogLevel::ERROR,
                        "Somehow got a invalid class while "
                        "registering a kernel device\n");
            return false;
    }
}

KDevice* get_kdevice(mode_t mode, dev_t dev)
{
    if (S_ISBLK(mode)) {
        if (!blkdev[major(dev)]) {
            return nullptr;
        }
        return blkdev[major(dev)]->get_kdevice(minor(dev));
    } else if (S_ISCHR(mode)) {
        if (!chrdev[major(dev)]) {
            return nullptr;
        }
        return chrdev[major(dev)]->get_kdevice(minor(dev));
    } else {
        return nullptr;
    }
}

Pair<int, void*> KDevice::open(const char*)
{
    return Pair<int, void*>(0, nullptr);
}

ssize_t KDevice::read(uint8_t*, size_t, off_t)
{
    return -EBADF;
}

ssize_t KDevice::read(uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

ssize_t KDevice::write(uint8_t*, size_t, off_t)
{
    return -EBADF;
}

ssize_t KDevice::write(uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}
} // namespace Filesystem
