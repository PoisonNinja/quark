#include <errno.h>
#include <fs/dev.h>
#include <fs/devnum.h>
#include <fs/ioctl.h>
#include <fs/stat.h>
#include <fs/tty.h>
#include <kernel.h>
#include <lib/murmur.h>
#include <lib/string.h>
#include <lib/unordered_map.h>

using namespace libcxx::placeholders;

namespace filesystem
{
namespace terminal
{
namespace
{
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

void tty_driver::handle_input(const uint8_t* buffer, size_t count)
{
    if (this->handler) {
        this->handler(buffer, count);
    }
}

void tty_driver::init_termios(struct termios& termios)
{
}

void tty_driver::set_event_handler(tty_event_handler_t h)
{
    assert(!this->handler);
    this->handler = h;
}

tty::tty(tty_driver* driver, struct termios& termios)
    : kdevice(CHR)
    , driver(driver)
    , termios(termios)
    , input_queue(4096)
    , output_queue(4096)
{
    driver->set_event_handler(libcxx::bind(&tty::handle_input, this, _1, _2));
}

int tty::ioctl(unsigned long request, char* argp)
{
    {
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

    if (!this->output_queue.empty()) {
        return POLLIN;
    }

    return 0;
}

ssize_t tty::read(uint8_t* buffer, size_t count, off_t /* offset */)
{
    if (!count)
        return 0;
    size_t read = 0;

    if (this->output_queue.empty()) {
        this->queue.wait(scheduler::wait_interruptible);
    }

    while (read < count) {
        if (this->output_queue.empty()) {
            return read;
        }
        buffer[read++] = this->output_queue.pop();
    }

    return count;
}

ssize_t tty::write_to_driver(const uint8_t* buffer, size_t size)
{
    libcxx::vector<uint8_t> real_buffer(size);
    for (size_t i = 0; i < size; i++) {
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
    return this->driver->write(real_buffer.data(), real_buffer.size());
}

ssize_t tty::write(const uint8_t* buffer, size_t count, off_t /* offset */)
{
    return this->write_to_driver(buffer, count);
}

ssize_t tty::handle_input(const uint8_t* buffer, size_t count)
{
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
            if (c == this->termios.c_cc[VERASE]) {
                if (!this->input_queue.empty()) {
                    this->input_queue.pop_back();
                    const uint8_t eraser[3] = {'\010', ' ', '\010'};
                    this->write_to_driver(eraser, 3);
                }
                continue;
            }
            if (!this->input_queue.full()) {
                this->input_queue.push(c);
            }
            if (this->termios.c_lflag & ECHO) {
                this->write_to_driver(reinterpret_cast<uint8_t*>(&c), 1);
            }
            if (c == '\n' ||
                (this->termios.c_cc[VEOL] && c == this->termios.c_cc[VEOL])) {
                dump_input();
                continue;
            }
        } else if (this->termios.c_lflag & ECHO) {
            this->write_to_driver(reinterpret_cast<uint8_t*>(&c), 1);
        } else {
            if (!this->output_queue.full()) {
                this->output_queue.push(c);
            }
            this->queue.wakeup();
        }
    }
    return i;
}

void tty::winch(const struct winsize* ws)
{
    this->ws = *ws;
    // TODO: Notify userspace through signals
}

ssize_t tty::dump_input()
{
    while (!this->input_queue.empty()) {
        if (!this->output_queue.full()) {
            this->output_queue.push(this->input_queue.pop());
        }
    }
    this->queue.wakeup();
    return 0;
}

tty* register_tty(tty_driver* driver, dev_t major, dev_t minor)
{
    if (!driver) {
        return nullptr;
    }
    struct termios kterm;
    driver->init_termios(kterm);
    tty* t = new tty(driver, kterm);
    if (major) {
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
