#include <errno.h>
#include <fs/dev.h>
#include <fs/ioctl.h>
#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <lib/unordered_map.h>

namespace filesystem
{
namespace terminal
{
namespace
{
dev_t tty_major = 0;

struct tty_list_node {
    libcxx::node<tty_list_node> node;
    tty* data;
    dev_t dev;
};
libcxx::list<tty_list_node, &tty_list_node::node> tty_list;
} // namespace

const char* init_cc =
    "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0";
const size_t num_init_cc = 18;

tty_driver::tty_driver()
{
}

int tty_driver::ioctl(unsigned long request, char* argp)
{
    // TODO: Probably should be ENOIOCTLCMD
    return -EINVAL;
}

int tty_driver::open(const char* name)
{
    return 0;
}

ssize_t tty_driver::write(const uint8_t* buffer, size_t count)
{
    // Act as a sink
    return count;
}

void tty_driver::init_termios(struct termios& termios)
{
}

tty::tty(tty_driver* driver, struct termios& termios)
    : kdevice(CHR)
    , driver(driver)
    , termios(termios)
    , itail(0)
    , head(0)
    , tail(0)
{
}

int tty::ioctl(unsigned long request, char* argp)
{
    {
        scoped_lock<mutex> mlock(this->meta_lock);
        switch (request) {
            case TCGETS:
                libcxx::memcpy(argp, &this->termios, sizeof(this->termios));
                return 0;
            case TCSETSW:
                // TODO: Discard input
            case TCSETSF:
            // TODO: Flush
            case TCSETS:
                if (this->termios.c_iflag & ICANON &&
                    !((reinterpret_cast<struct termios*>(argp))->c_iflag &
                      ICANON)) {
                    // Switching out of ICANON
                    this->dump_input();
                }
                libcxx::memcpy(&this->termios, argp, sizeof(this->termios));
                return 0;
            case TIOCGWINSZ:
                libcxx::memcpy(argp, &this->ws, sizeof(this->ws));
                return 0;
        }
    }
    // We don't know how to handle it, punt it to the driver
    return this->driver->ioctl(request, argp);
}

int tty::open(const char* name)
{
    return this->driver->open(name);
}

int tty::poll(filesystem::poll_register_func_t& callback)
{
    callback(this->queue);

    scoped_lock<mutex> lock(this->buffer_lock);
    if (this->head != this->tail) {
        return POLLIN;
    }

    return 0;
}

ssize_t tty::read(uint8_t* buffer, size_t count, off_t /* offset */)
{
    if (!count)
        return 0;
    size_t read = 0;
    this->buffer_lock.lock();

    if (this->head == this->tail) {
        this->buffer_lock.unlock();
        this->queue.wait(scheduler::wait_interruptible);
        this->buffer_lock.lock();
    }

    while (read < count) {
        if (this->tail == this->head) {
            this->buffer_lock.unlock();
            return read;
        }
        buffer[read++] = this->buffer[this->tail++ % 4096];
    }

    this->buffer_lock.unlock();
    return count;
}

ssize_t tty::write(const uint8_t* buffer, size_t count, off_t /* offset */)
{
    this->meta_lock.lock();
    libcxx::vector<uint8_t> real_buffer(count);
    for (size_t i = 0; i < count; i++) {
        if (this->termios.c_oflag & ONLCR && buffer[i] == '\n') {
            real_buffer.push_back('\n');
            real_buffer.push_back('\r');
            continue;
        }
        if (this->termios.c_oflag & ONLRET && buffer[i] == '\r') {
            continue;
        }
        real_buffer.push_back(buffer[i]);
    }
    this->meta_lock.unlock();
    return this->driver->write(real_buffer.data(), real_buffer.size());
}

ssize_t tty::notify(const uint8_t* buffer, size_t count)
{
    this->meta_lock.lock();
    size_t i;
    for (i = 0; i < count; i++) {
        char c = buffer[i];
        if (this->termios.c_iflag & ISTRIP) {
            // Strip the eigth bit
            c &= 0b01111111;
        }
        if (this->termios.c_iflag & IGNCR && c == '\r') {
            // Drop carriage returns
            continue;
        }
        if (this->termios.c_iflag & ICRNL && c == '\r') {
            // Convert carriage returns to newlines
            c = '\n';
        }
        if (this->termios.c_iflag & INLCR && c == '\n') {
            // Convert newlines to carriage returns
            c = '\r';
        }
        if (this->termios.c_lflag & ICANON) {
            {
                scoped_lock<mutex> ilock(this->ibuffer_lock);
                if (c == this->termios.c_cc[VERASE]) {
                    if (this->itail) {
                        this->ibuffer[--this->itail] = '\0';
                        const uint8_t eraser[3]      = {'\010', ' ', '\010'};
                        this->meta_lock.unlock();
                        this->driver->write(eraser, 3);
                        this->meta_lock.lock();
                    }
                    continue;
                }
                this->ibuffer[this->itail++] = c;
            }

            if (this->termios.c_lflag & ECHO) {
                this->meta_lock.unlock();
                this->driver->write(&buffer[i], 1);
                this->meta_lock.lock();
            }
            if (c == '\n' ||
                (this->termios.c_cc[VEOL] && c == this->termios.c_cc[VEOL])) {
                dump_input();
                continue;
            }
        } else if (this->termios.c_lflag & ECHO) {
            this->meta_lock.unlock();
            this->driver->write(&buffer[i], 1);
            this->meta_lock.lock();
        } else {
            {
                scoped_lock<mutex> lock(this->buffer_lock);
                this->buffer[this->head++ % 4096] = buffer[i];
            }
            this->queue.wakeup();
        }
    }
    this->meta_lock.unlock();
    return i;
}

void tty::winch(const struct winsize* ws)
{
    scoped_lock<mutex> mlock(this->meta_lock);
    this->ws = *ws;
    // TODO: Notify userspace through signals
}

ssize_t tty::dump_input()
{
    {
        scoped_lock<mutex> lock(this->buffer_lock);
        scoped_lock<mutex> ilock(this->ibuffer_lock);
        for (size_t i = 0; i < this->itail; i++) {
            this->buffer[this->head++ % 4096] = this->ibuffer[i];
        }
        this->itail = 0;
    }
    this->queue.wakeup();
    return 0;
}

tty* register_tty(tty_driver* driver, dev_t major, dev_t minor, unsigned flags)
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
    tty* t = new tty(driver, kterm);
    if (!(flags & tty_no_register)) {
        register_kdevice(filesystem::CHR, major, minor, t);
        tty_list_node* tty_node = new tty_list_node;
        tty_node->dev           = mkdev(major, minor);
        tty_node->data          = t;
        tty_list.push_front(*tty_node);
    }
    return t;
}

tty* get_tty(dev_t major, dev_t minor)
{
    for (auto& t : tty_list) {
        if (t.dev == mkdev(major, minor)) {
            return t.data;
        }
    }
    return nullptr;
}

void init()
{
    auto tty_major = filesystem::locate_class(filesystem::CHR);
    register_class(CHR, tty_major);
}
} // namespace terminal
} // namespace filesystem
