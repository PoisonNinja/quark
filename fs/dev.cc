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

ssize_t next_free_major(KDevice* arr[max_major])
{
    for (size_t i = 0; i < max_major; i++) {
        if (!arr[i]) {
            return i;
        }
    }
    return -1;
}
}  // namespace

bool reserve_class(DeviceClass c, dev_t major)
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
        return blkdev[major(dev)]->get_kdevice(minor(dev));
    } else if (S_ISCHR(mode)) {
        return chrdev[major(dev)]->get_kdevice(minor(dev));
    } else {
        return nullptr;
    }
}

}  // namespace Filesystem
