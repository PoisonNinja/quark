#include <errno.h>
#include <fs/dev.h>
#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>
#include <lib/murmur.h>
#include <lib/unordered_map.h>

namespace
{
dev_t tty_major = 0;
} // namespace

namespace filesystem
{
namespace tty
{
tty_driver::tty_driver()
    : core(nullptr)
{
}

libcxx::pair<int, void*> tty_driver::open(const char* name)
{
    return libcxx::pair<int, void*>(0, nullptr);
}

ssize_t tty_driver::write(const uint8_t* buffer, size_t count)
{
    // Act as a sink
    return count;
}

void tty_driver::set_core(tty_core* core)
{
    if (this->core) {
        log::printk(log::log_level::WARNING,
                    "tty: Uhh, you already have a core driver, this is "
                    "probably not what you want\n");
    }
    this->core = core;
}

tty_core::tty_core(tty_driver* driver)
    : kdevice(CHR)
    , driver(driver)
{
}

int tty_core::ioctl(unsigned long request, char* argp, void* cookie)
{
}

libcxx::pair<int, void*> tty_core::open(const char* name)
{
    return this->driver->open(name);
}

int tty_core::poll(filesystem::poll_register_func_t& callback, void* cookie)
{
}

ssize_t tty_core::read(uint8_t* buffer, size_t count, off_t /* offset */,
                       void* cookie)
{
}

ssize_t tty_core::write(const uint8_t* buffer, size_t count, off_t /* offset */,
                        void* cookie)
{
}

ssize_t tty_core::notify(const uint8_t* buffer, size_t count)
{
}

tty_core* register_tty(tty_driver* driver, dev_t major, dev_t minor)
{
    if (!driver) {
        return nullptr;
    }
    if (!major) {
        major = tty_major;
    } else {
        register_class(filesystem::CHR, major);
    }
    tty_core* tty = new tty_core(driver);
    driver->set_core(tty);
    register_kdevice(filesystem::CHR, major, tty);
    return tty;
}

void init()
{
    auto tty_major = filesystem::locate_class(filesystem::CHR);
    register_class(CHR, tty_major);
}
} // namespace tty
} // namespace filesystem
