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

ssize_t tty::read(uint8_t* /*buffer*/, size_t /*size*/, void* /* cookie */)
{
    return -ENOSYS;
}

ssize_t tty::write(const uint8_t* /*buffer*/, size_t /*size*/,
                   void* /* cookie */)
{
    return -ENOSYS;
}

class TTYDevice : public filesystem::kdevice
{
public:
    TTYDevice(tty* driver);

    int ioctl(unsigned long request, char* argp, void* cookie) override;

    libcxx::pair<int, void*> open(const char* name) override;

    ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                 void* cookie) override;
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                  void* cookie) override;

private:
    tty* t;
};

TTYDevice::TTYDevice(tty* tty)
    : kdevice(CHR)
    , t(tty)
{
}

int TTYDevice::ioctl(unsigned long request, char* argp, void* cookie)
{
    return this->t->ioctl(request, argp, cookie);
}

libcxx::pair<int, void*> TTYDevice::open(const char* name)
{
    return this->t->open(name);
}

ssize_t TTYDevice::read(uint8_t* buffer, size_t count, off_t /* offset */,
                        void* cookie)
{
    return this->t->read(buffer, count, cookie);
}

ssize_t TTYDevice::write(const uint8_t* buffer, size_t count,
                         off_t /* offset */, void* cookie)
{
    return this->t->write(buffer, count, cookie);
}

bool register_tty(dev_t major, tty* driver)
{
    TTYDevice* tty = new TTYDevice(driver);
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
