#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <drivers/input/input.h>
#include <fs/dev.h>
#include <kernel.h>
#include <kernel/init.h>
#include <lib/functional.h>
#include <proc/wq.h>

using namespace libcxx::placeholders;

namespace
{
constexpr size_t buffer_size = 1024;

class intel8042 : public input::device
{
public:
    intel8042();

private:
    void handler(int, void*, struct interrupt_context*);
};

intel8042::intel8042()
    : device()
{
    interrupt::handler* h = new interrupt::handler(
        libcxx::bind(&intel8042::handler, this, _1, _2, _3), "keyboard",
        reinterpret_cast<void*>(this));
    interrupt::register_handler(interrupt::irq_to_interrupt(1), *h);
    /*
     * Read a single byte to allow the controller to continue sending input.
     * We're going to lose this byte though.
     */
    inb(0x60);
}

void intel8042::handler(int, void* dev_id, struct interrupt_context*)
{
    unsigned scancode = inb(0x60);
    int is_press      = 1;
    if (scancode & 0x80) {
        is_press = 0;
        scancode &= ~0x80;
    }
    input::report_event(this, input::dtk_event_type::key, scancode, is_press);
}
} // namespace

namespace
{
int init()
{
    intel8042* d = new intel8042();
    input::register_device(d);
    return 0;
}
DEVICE_INITCALL(init);
} // namespace
