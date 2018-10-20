#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <drivers/input/i8042.h>
#include <fs/dev.h>
#include <kernel.h>
#include <kernel/init.h>
#include <proc/sched.h>

namespace
{
constexpr size_t buffer_size = 1024;

class i8042Driver : public Filesystem::KDevice
{
public:
    i8042Driver();

    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                         void* cookie) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset,
                          void* cookie) override;

    virtual bool seekable() override;

private:
    static void handler(int, void*, struct InterruptContext*);
    char buffer[buffer_size];
    size_t head, tail;
    Scheduler::WaitQueue queue;
};

i8042Driver::i8042Driver()
    : KDevice(Filesystem::CHR)
    , head(0)
    , tail(0)
{
    Interrupt::Handler* h = new Interrupt::Handler(
        handler, "keyboard", reinterpret_cast<void*>(this));
    Interrupt::register_handler(Interrupt::irq_to_interrupt(1), *h);
}

ssize_t i8042Driver::read(uint8_t* buffer, size_t count, off_t /*offset*/,
                          void*)
{
    size_t read = 0;
    while (read < count) {
        if (this->head == this->tail) {
            this->queue.wait(Scheduler::wait_interruptible);
        }
        buffer[read++] = this->buffer[tail++ % buffer_size];
    }
    return read;
}

ssize_t i8042Driver::write(uint8_t*, size_t, off_t, void*)
{
    return 0;
}

void i8042Driver::handler(int, void* dev_id, struct InterruptContext*)
{
    i8042Driver* driver = reinterpret_cast<i8042Driver*>(dev_id);
    driver->buffer[driver->head++ % buffer_size] = inb(0x60);
    driver->queue.wakeup();
}

bool i8042Driver::seekable()
{
    return false;
}
} // namespace

namespace i8042
{
int init()
{
    dev_t major = Filesystem::locate_class(Filesystem::CHR);
    Filesystem::register_class(Filesystem::CHR, major);
    i8042Driver* d = new i8042Driver();
    Filesystem::register_kdevice(Filesystem::CHR, major, d);
    return 0;
}
DEVICE_INITCALL(init);
} // namespace i8042
