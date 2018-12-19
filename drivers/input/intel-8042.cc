#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <fs/dev.h>
#include <kernel.h>
#include <kernel/init.h>
#include <lib/functional.h>
#include <proc/sched.h>

using namespace libcxx::placeholders;

namespace
{
constexpr size_t buffer_size = 1024;

class Intel8042 : public Filesystem::KDevice
{
public:
    Intel8042();

    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                         void* cookie) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset,
                          void* cookie) override;

    virtual bool seekable() override;

private:
    void handler(int, void*, struct InterruptContext*);
    char buffer[buffer_size];
    size_t head, tail;
    Scheduler::WaitQueue queue;
};

Intel8042::Intel8042()
    : KDevice(Filesystem::CHR)
    , head(0)
    , tail(0)
{
    Interrupt::Handler* h = new Interrupt::Handler(
        libcxx::bind(&Intel8042::handler, this, _1, _2, _3), "keyboard",
        reinterpret_cast<void*>(this));
    Interrupt::register_handler(Interrupt::irq_to_interrupt(1), *h);
}

ssize_t Intel8042::read(uint8_t* buffer, size_t count, off_t /*offset*/, void*)
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

ssize_t Intel8042::write(uint8_t*, size_t, off_t, void*)
{
    return 0;
}

void Intel8042::handler(int, void* dev_id, struct InterruptContext*)
{
    this->buffer[this->head++ % buffer_size] = inb(0x60);
    Log::printk(Log::LogLevel::INFO, "Key event, %p\n", this);
    this->queue.wakeup();
}

bool Intel8042::seekable()
{
    return false;
}
} // namespace

namespace
{
int init()
{
    dev_t major = Filesystem::locate_class(Filesystem::CHR);
    Filesystem::register_class(Filesystem::CHR, major);
    Intel8042* d = new Intel8042();
    Filesystem::register_kdevice(Filesystem::CHR, major, d);
    return 0;
}
DEVICE_INITCALL(init);
} // namespace