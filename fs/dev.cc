#include <fs/dev.h>
#include <fs/stat.h>
#include <kernel.h>

namespace Filesystem
{
namespace
{
constexpr size_t max_major = 256;

KDevice* chrdev[max_major] = {nullptr};
KDevice* blkdev[max_major] = {nullptr};

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

bool register_kdevice(DeviceClass c, dev_t major, KDevice* kdev)
{
    switch (c) {
        case BLK:
            if (blkdev[major]) {
                Log::printk(Log::WARNING,
                            "Block device already exists with major %llX\n",
                            major);
                return false;
            }
            blkdev[major] = kdev;
            return true;
        case CHR:
            if (chrdev[major]) {
                Log::printk(Log::WARNING,
                            "Character device already exists with major %llX\n",
                            major);
                return false;
            }
            chrdev[major] = kdev;
            return true;
        default:
            Log::printk(Log::ERROR, "Somehow got a invalid class while "
                                    "registering a kernel device\n");
            return false;
    }
}

KDevice* get_kdevice(mode_t mode, dev_t dev)
{
    if (S_ISBLK(mode)) {
        return blkdev[major(dev)];
    } else if (S_ISCHR(mode)) {
        return chrdev[major(dev)];
    } else {
        return nullptr;
    }
}

}  // namespace Filesystem