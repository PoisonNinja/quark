#include <errno.h>
#include <fs/dev.h>
#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>

namespace
{
dev_t tty_major = 0;
}

namespace filesystem
{
namespace tty
{
tty::tty()
{
}

tty::~tty()
{
}

libcxx::pair<int, void*> tty::open(const char* name)
{
    return libcxx::pair<int, void*>(0, nullptr);
}

int tty::ioctl(unsigned long request, char* argp, void* cookie)
{
    return 0;
}

int tty::poll(filesystem::poll_register_func_t& callback, void* cookie)
{
    return POLLIN;
}

ssize_t tty::read(uint8_t* /*buffer*/, size_t /*size*/, void* /* cookie */)
{
    return -ENOSYS;
}

ssize_t tty::write(const uint8_t* /*buffer*/, size_t /*size*/,
                   void* /* cookie */)
{
    return -ENOSYS;
}

tty_device::tty_device(tty* tty)
    : kdevice(CHR)
    , t(tty)
{
}

int tty_device::ioctl(unsigned long request, char* argp, void* cookie)
{
    return this->t->ioctl(request, argp, cookie);
}

libcxx::pair<int, void*> tty_device::open(const char* name)
{
    return this->t->open(name);
}

int tty_device::poll(filesystem::poll_register_func_t& callback, void* cookie)
{
    return this->t->poll(callback, cookie);
}

ssize_t tty_device::read(uint8_t* buffer, size_t count, off_t /* offset */,
                         void* cookie)
{
    return this->t->read(buffer, count, cookie);
}

ssize_t tty_device::write(const uint8_t* buffer, size_t count,
                          off_t /* offset */, void* cookie)
{
    return this->t->write(buffer, count, cookie);
}

bool register_tty(dev_t major, tty* driver)
{
    tty_device* tty = new tty_device(driver);
    register_kdevice(CHR, (major) ? major : tty_major, tty);
    return true;
}

void init()
{
    auto tty_major = filesystem::locate_class(filesystem::CHR);
    register_class(CHR, tty_major);
}
} // namespace tty
} // namespace filesystem
