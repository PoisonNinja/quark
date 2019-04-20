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
const char* init_cc =
    "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0";
const size_t num_init_cc = 18;

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

void tty_driver::init_termios(struct termios& termios)
{
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

tty_core::tty_core(tty_driver* driver, struct termios& termios)
    : kdevice(CHR)
    , driver(driver)
    , termios(termios)
    , itail(0)
    , head(0)
    , tail(0)
{
}

int tty_core::ioctl(unsigned long request, char* argp, void* cookie)
{
    return 0;
}

libcxx::pair<int, void*> tty_core::open(const char* name)
{
    return this->driver->open(name);
}

int tty_core::poll(filesystem::poll_register_func_t& callback, void* cookie)
{
    callback(this->queue);
    if (this->head != this->tail) {
        return POLLIN;
    }
    return 0;
}

ssize_t tty_core::read(uint8_t* buffer, size_t count, off_t /* offset */,
                       void* cookie)
{
    if (!count)
        return 0;
    size_t read = 0;
    if (this->head == this->tail) {
        this->queue.wait(scheduler::wait_interruptible);
    }
    while (read < count) {
        if (this->tail == this->head) {
            return read;
        }
        buffer[read++] = this->buffer[this->tail++ % 4096];
    }
    return count;
}

ssize_t tty_core::write(const uint8_t* buffer, size_t count, off_t /* offset */,
                        void* cookie)
{
    return this->driver->write(buffer, count);
}

ssize_t tty_core::notify(const uint8_t* buffer, size_t count)
{
    size_t i;
    for (i = 0; i < count; i++) {
        char c = buffer[i];
        if (this->termios.c_lflag & ICANON) {
            if (c == this->termios.c_cc[VERASE]) {
                if (this->itail) {
                    this->ibuffer[--this->itail] = '\0';
                    const uint8_t eraser[3]      = {'\010', ' ', '\010'};
                    this->driver->write(eraser, 3);
                }
                continue;
            }
            this->ibuffer[this->itail++] = c;
            if (this->termios.c_lflag & ECHO) {
                this->driver->write(&buffer[i], 1);
            }
            if (c == '\n' ||
                (this->termios.c_cc[VEOL] && c == this->termios.c_cc[VEOL])) {
                dump_input();
                continue;
            }
        } else if (this->termios.c_lflag & ECHO) {
            this->driver->write(&buffer[i], 1);
        } else {
            this->buffer[this->head++ % 4096] = buffer[i];
            this->queue.wakeup();
        }
    }
    return i;
}

ssize_t tty_core::dump_input()
{
    for (size_t i = 0; i < this->itail; i++) {
        this->buffer[this->head++ % 4096] = this->ibuffer[i];
    }
    this->itail = 0;
    this->queue.wakeup();
    return 0;
}

tty_core* register_tty(tty_driver* driver, dev_t major, dev_t minor,
                       unsigned flags)
{
    if (!driver) {
        return nullptr;
    }
    if (!(flags & tty_no_register)) {
        if (!major) {
            major = tty_major;
        } else {
            register_class(filesystem::CHR, major);
        }
    }
    struct termios kterm;
    driver->init_termios(kterm);
    tty_core* tty = new tty_core(driver, kterm);
    driver->set_core(tty);
    if (!(flags & tty_no_register)) {
        register_kdevice(filesystem::CHR, major, tty);
    }
    return tty;
}

void init()
{
    auto tty_major = filesystem::locate_class(filesystem::CHR);
    register_class(CHR, tty_major);
}
} // namespace tty
} // namespace filesystem
