#include <errno.h>
#include <fs/dev.h>
#include <fs/stat.h>
#include <kernel.h>

namespace filesystem
{
namespace
{
constexpr size_t max_major = 256;
constexpr size_t max_minor = 256;

struct KDeviceClass {
    kdevice* minors[max_minor];
    kdevice* get_kdevice(int minor);
    void add_kdevice(kdevice* kdevice);
};

void KDeviceClass::add_kdevice(kdevice* kdevice)
{
    for (size_t i = 0; i < max_minor; i++) {
        if (!minors[i]) {
            minors[i] = kdevice;
            return;
        }
    }
}

kdevice* KDeviceClass::get_kdevice(int minor)
{
    return minors[minor];
}

KDeviceClass* chrdev[max_major] = {nullptr};
KDeviceClass* blkdev[max_major] = {nullptr};
} // namespace

dev_t locate_class(device_class c)
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

bool register_class(device_class c, dev_t major)
{
    switch (c) {
        case BLK:
            if (blkdev[major]) {
                log::printk(log::log_level::WARNING,
                            "Block device already exists with major %llX\n",
                            major);
                return false;
            }
            blkdev[major] = new KDeviceClass();
            return true;
        case CHR:
            if (chrdev[major]) {
                log::printk(log::log_level::WARNING,
                            "Character device already exists with major %llX\n",
                            major);
                return false;
            }
            chrdev[major] = new KDeviceClass();
            return true;
        default:
            log::printk(log::log_level::ERROR,
                        "Somehow got a invalid class while "
                        "registering a kernel device\n");
            return false;
    }
}

bool register_kdevice(device_class c, dev_t major, kdevice* kdev)
{
    switch (c) {
        case BLK:
            if (!blkdev[major]) {
                log::printk(log::log_level::WARNING,
                            "Block device with major %llX not found\n", major);
                return false;
            }
            blkdev[major]->add_kdevice(kdev);
            return true;
        case CHR:
            if (!chrdev[major]) {
                log::printk(log::log_level::WARNING,
                            "Character device with major %llX not found\n",
                            major);
                return false;
            }
            chrdev[major]->add_kdevice(kdev);
            return true;
        default:
            log::printk(log::log_level::ERROR,
                        "Somehow got a invalid class while "
                        "registering a kernel device\n");
            return false;
    }
}

kdevice* get_kdevice(mode_t mode, dev_t dev)
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

libcxx::pair<int, void*> kdevice::open(const char*)
{
    return libcxx::pair<int, void*>(0, nullptr);
}

int kdevice::ioctl(unsigned long, char*, void* cookie)
{
    return -ENOSYS;
}

int kdevice::poll(poll_register_func_t& callback, void* cookie)
{
    // TODO: Correct?
    return POLLIN;
}

ssize_t kdevice::read(uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

ssize_t kdevice::write(const uint8_t*, size_t, off_t, void*)
{
    return -EBADF;
}

bool kdevice::seekable()
{
    return true;
}
} // namespace filesystem
