#include <errno.h>
#include <fs/dev.h>
#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>

namespace
{
dev_t tty_major = 0;
}

namespace Filesystem
{
namespace TTY
{
TTY::TTY()
{
}

TTY::~TTY()
{
}

libcxx::pair<int, void*> TTY::open(const char* name)
{
    return libcxx::pair<int, void*>(0, nullptr);
}

int TTY::ioctl(unsigned long request, char* argp, void* cookie)
{
    return 0;
}

ssize_t TTY::read(uint8_t* /*buffer*/, size_t /*size*/, void* /* cookie */)
{
    return -ENOSYS;
}

ssize_t TTY::write(uint8_t* /*buffer*/, size_t /*size*/, void* /* cookie */)
{
    return -ENOSYS;
}

class TTYDevice : public Filesystem::KDevice
{
public:
    TTYDevice(TTY* driver);

    int ioctl(unsigned long request, char* argp, void* cookie) override;

    libcxx::pair<int, void*> open(const char* name) override;

    ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                 void* cookie) override;
    ssize_t write(uint8_t* buffer, size_t count, off_t offset,
                  void* cookie) override;

private:
    TTY* tty;
};

TTYDevice::TTYDevice(TTY* tty)
    : KDevice(CHR)
    , tty(tty)
{
}

int TTYDevice::ioctl(unsigned long request, char* argp, void* cookie)
{
    return this->tty->ioctl(request, argp, cookie);
}

libcxx::pair<int, void*> TTYDevice::open(const char* name)
{
    return this->tty->open(name);
}

ssize_t TTYDevice::read(uint8_t* buffer, size_t count, off_t /* offset */,
                        void* cookie)
{
    return this->tty->read(buffer, count, cookie);
}

ssize_t TTYDevice::write(uint8_t* buffer, size_t count, off_t /* offset */,
                         void* cookie)
{
    return this->tty->write(buffer, count, cookie);
}

bool register_tty(dev_t major, TTY* driver)
{
    TTYDevice* tty = new TTYDevice(driver);
    register_kdevice(CHR, (major) ? major : tty_major, tty);
    return true;
}

void init()
{
    auto tty_major = Filesystem::locate_class(Filesystem::CHR);
    register_class(CHR, tty_major);
}
} // namespace TTY
} // namespace Filesystem
