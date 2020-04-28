#include <drivers/input/input.h>
#include <fs/devnum.h>
#include <fs/tty.h>
#include <fs/vtty/vtty.h>
#include <kernel/init.h>
#include <lib/printf.h>
#include <lib/string.h>

namespace input
{
namespace
{
libcxx::list<input_handler, &input_handler::node> input_handlers;
} // namespace

input_kdevice::input_kdevice()
    : kdevice(filesystem::CHR)
    , head(0)
    , tail(0)
{
}

int input_kdevice::poll(filesystem::poll_register_func_t& callback)
{
    /*
     * Register this thread with the queue
     */
    callback(this->queue);
    if (this->head != this->tail) {
        return POLLIN;
    }
    return 0;
}

ssize_t input_kdevice::read(uint8_t* buffer, size_t count, off_t /*offset*/)
{
    size_t read = 0;
    while (read + sizeof(event_data) <= count) {
        if (this->head == this->tail) {
            this->queue.wait(scheduler::wait_interruptible);
        }
        libcxx::memcpy(buffer + read, &this->buffer[tail % buffer_size],
                       sizeof(event_data));
        tail += 1;
        read += sizeof(event_data);
    }
    return read;
}

bool input_kdevice::seekable()
{
    return false;
}

void input_kdevice::append(dtk_event_type type, unsigned code, int val)
{
    event_data* ptr = &this->buffer[head % buffer_size];
    ptr->type       = static_cast<unsigned>(type);
    ptr->code       = code;
    ptr->value      = val;
    this->queue.wakeup();
}

device::device()
{
}

int device::event(ktd_event_type type, unsigned code, int value)
{
    return 0;
}

void device::set_kdevice(input_kdevice* k)
{
    this->kdev = k;
}

input_kdevice* device::get_kdevice()
{
    return this->kdev;
}

bool register_handler(input_handler& handler)
{
    input_handlers.push_back(handler);
    return true;
}

bool register_device(device* dev)
{
    input_kdevice* k = new input_kdevice;
    dev->set_kdevice(k);
    filesystem::register_kdevice(filesystem::CHR, input_major, k);
    return true;
}

void report_event(device* dev, dtk_event_type type, unsigned code, int value)
{
    if (dev == nullptr) {
        return;
    }
    input_kdevice* k = dev->get_kdevice();
    k->append(type, code, value);
    for (auto& handler : input_handlers) {
        handler.handler(type, code, value);
    }
}

namespace
{
int init()
{
    filesystem::register_class(filesystem::CHR, input_major);
    return 0;
}
SUBSYS_INITCALL(init);
} // namespace
} // namespace input
