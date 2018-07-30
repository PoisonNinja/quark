#pragma once

#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <drivers/input/i8042.h>
#include <fs/dev.h>
#include <kernel.h>
#include <proc/sched.h>

namespace
{
/* KBDUS means US Keyboard Layout. This is a scancode table
 *  used to layout a standard US keyboard. I have left some
 *  comments in to give you an idea of what key is what, even
 *  though I set it's array index to 0. You can change that to
 *  whatever you want using a macro, if you wish! */
unsigned char kbdus[128] = {
    0,    27,  '1', '2', '3',  '4', '5', '6', '7',  '8', /* 9 */
    '9',  '0', '-', '=', '\b',                           /* Backspace */
    '\t',                                                /* Tab */
    'q',  'w', 'e', 'r',                                 /* 19 */
    't',  'y', 'u', 'i', 'o',  'p', '[', ']', '\n',      /* Enter key */
    0,                                                   /* 29   - Control */
    'a',  's', 'd', 'f', 'g',  'h', 'j', 'k', 'l',  ';', /* 39 */
    '\'', '`', 0,                                        /* Left shift */
    '\\', 'z', 'x', 'c', 'v',  'b', 'n',                 /* 49 */
    'm',  ',', '.', '/', 0,                              /* Right shift */
    '*',  0,                                             /* Alt */
    ' ',                                                 /* Space bar */
    0,                                                   /* Caps lock */
    0,                                                   /* 59 - F1 key ... > */
    0,    0,   0,   0,   0,    0,   0,   0,   0,         /* < ... F10 */
    0,                                                   /* 69 - Num lock*/
    0,                                                   /* Scroll Lock */
    0,                                                   /* Home key */
    0,                                                   /* Up Arrow */
    0,                                                   /* Page Up */
    '-',  0,                                             /* Left Arrow */
    0,    0,                                             /* Right Arrow */
    '+',  0,                                             /* 79 - End key*/
    0,                                                   /* Down Arrow */
    0,                                                   /* Page Down */
    0,                                                   /* Insert Key */
    0,                                                   /* Delete Key */
    0,    0,   0,   0,                                   /* F11 Key */
    0,                                                   /* F12 Key */
    0, /* All other keys are undefined */
};

constexpr size_t buffer_size = 1024;

class i8042Driver : public Filesystem::KDevice
{
public:
    i8042Driver();

    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset) override;

private:
    static void handler(int, void*, struct InterruptContext*);
    char buffer[buffer_size];
    size_t head, tail;
    Scheduler::WaitQueue queue;
};

i8042Driver::i8042Driver() : KDevice(Filesystem::CHR), head(0), tail(0)
{
    Interrupt::Handler* h = new Interrupt::Handler(
        handler, "keyboard", reinterpret_cast<void*>(this));
    Interrupt::register_handler(Interrupt::irq_to_interrupt(1), *h);
}

ssize_t i8042Driver::read(uint8_t* buffer, size_t count, off_t offset)
{
    size_t read = 0;
    while (read < count) {
        if (this->head == this->tail) {
            Log::printk(Log::LogLevel::INFO, "Sleeping\n");
            this->queue.wait(Scheduler::wait_interruptible);
        } else {
            Log::printk(Log::LogLevel::INFO, "We have existing data\n");
        }
        Log::printk(Log::LogLevel::INFO, "Reading\n");
        buffer[read++] = this->buffer[tail++ % buffer_size];
    }
}

ssize_t i8042Driver::write(uint8_t*, size_t, off_t)
{
    return 0;
}

void i8042Driver::handler(int, void* dev_id, struct InterruptContext*)
{
    Log::printk(Log::LogLevel::WARNING, "Entering i8042 handler\n");
    i8042Driver* driver = reinterpret_cast<i8042Driver*>(dev_id);
    uint8_t byte = inb(0x60);
    if (!(byte & 0x80)) {
        driver->buffer[driver->head++ % buffer_size] = kbdus[byte];
        driver->queue.wakeup();
    }
    Log::printk(Log::LogLevel::WARNING, "Exiting i8042 handler\n");
}

}  // namespace

namespace i8042
{
void init()
{
    Filesystem::reserve_class(Filesystem::CHR, 1);
    i8042Driver* d = new i8042Driver();
    Filesystem::register_kdevice(Filesystem::CHR, 1, d);
}
}  // namespace i8042
