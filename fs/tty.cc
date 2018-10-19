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

ssize_t TTY::read(uint8_t* /*buffer*/, size_t /*size*/)
{
    return -ENOSYS;
}

ssize_t TTY::write(uint8_t* /*buffer*/, size_t /*size*/)
{
    return -ENOSYS;
}

class TTYDevice : public Filesystem::KDevice
{
public:
    TTYDevice(TTY* driver);

    Pair<int, void*> open(const char* name) override;

    ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    ssize_t write(uint8_t* buffer, size_t count, off_t offset) override;

private:
    TTY* driver;
};

TTYDevice::TTYDevice(TTY* driver)
    : KDevice(CHR)
    , driver(driver)
{
}

Pair<int, void*> TTYDevice::open(const char* name)
{
    return Pair<int, void*>(0, nullptr);
}

ssize_t TTYDevice::read(uint8_t* buffer, size_t count, off_t /* offset */)
{
    return this->driver->read(buffer, count);
}

ssize_t TTYDevice::write(uint8_t* buffer, size_t count, off_t /* offset */)
{
    return this->driver->write(buffer, count);
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
