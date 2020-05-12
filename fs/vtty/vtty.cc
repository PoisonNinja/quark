#include <arch/drivers/io.h>
#include <drivers/input/codes.h>
#include <fs/devnum.h>
#include <fs/vtty/vtty.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/virtual.h>
#include <mm/vmalloc.h>

using namespace libcxx::placeholders;
using namespace input;

namespace filesystem
{
namespace terminal
{
namespace
{
const addr_t VGA_BUFFER_BASE = 0xB8000;
const size_t VGA_BUFFER_SIZE = 0x8000;
const int VGA_HEIGHT         = 25;
const int VGA_WIDTH          = 80;

constexpr int NUM_VTTYS = 7;

uint16_t* vga_buffer = nullptr;

vtty* vttys[NUM_VTTYS + 1];
int current_tty = -1;

void update_cursor(int col, int row)
{
    uint16_t position = (row * VGA_WIDTH) + col;

    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void enable_cursor()
{
    outb(0x3D4, 0x09); // set maximum scan line register to 15
    outb(0x3D5, 0x0F);

    outb(0x3D4, 0x0B); // set the cursor end line to 15
    outb(0x3D5, 0x0F);

    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x0E);
}
} // namespace

vtty::vtty(int id)
    : id(id)
    , x(0)
    , y(0)
    , bg(0)
    , fg(15)
{
    handler = new input::input_handler(
        libcxx::bind(&vtty::handle_kb, this, _1, _2, _3));
    input::register_handler(*this->handler);
    libcxx::memset(this->internal_buffer, 0, sizeof(this->internal_buffer));
    for (int i = 0; i < 80 * 25; i++) {
        internal_buffer[i] = 15 << 8;
    }
}

int vtty::ioctl(unsigned long request, char* argp)
{
    return 0;
}

int vtty::open(const char* name)
{
    return 0;
}

ssize_t vtty::write(const uint8_t* buffer, size_t count)
{
    for (const uint8_t* written = buffer; written < buffer + count; written++) {
        char val = *written;

        if (val == '\n') {
            y++;
        } else if (val == '\r') {
            x = 0;
        } else if (val == '\b') {
            if (x)
                x--;
        } else if (val == '\e') {
            val = written[3];
            if (written[2] == '3') {
                switch (val) {
                    case '0':
                        fg = 0; // Black
                        break;
                    case '1':
                        fg = 4; // Red
                        break;
                    case '2':
                        fg = 2; // Green
                        break;
                    case '3':
                        fg = 14; // Yellow
                        break;
                    case '4':
                        fg = 1; // Blue
                        break;
                    case '5':
                        fg = 5; // Magenta
                        break;
                    case '6':
                        fg = 3; // Cyan
                        break;
                    case '7':
                        fg = 7; // Light gray
                        break;
                    case '9':
                        fg = 15;
                        break;
                }
            } else if (written[2] == '9') {
                switch (val) {
                    case '0':
                        fg = 8; // Dark gray
                        break;
                    case '1':
                        fg = 12; // Light Red
                        break;
                    case '2':
                        fg = 10; // Light Green
                        break;
                    case '3':
                        fg = 14; // Yellow
                        break;
                    case '4':
                        fg = 9; // Light blue
                        break;
                    case '5':
                        fg = 13; // Light Magenta
                        break;
                    case '6':
                        fg = 11; // Light Cyan
                        break;
                    case '7':
                        fg = 15; // White
                        break;
                }
            } else if (written[2] == '4') {
                switch (val) {
                    case '0':
                        bg = 0; // Black
                        break;
                    case '1':
                        bg = 4; // Red
                        break;
                    case '2':
                        bg = 2; // Green
                        break;
                    case '3':
                        bg = 14; // Yellow
                        break;
                    case '4':
                        bg = 1; // Blue
                        break;
                    case '5':
                        bg = 5; // Magenta
                        break;
                    case '6':
                        bg = 3; // Cyan
                        break;
                    case '7':
                        bg = 7; // Light gray
                        break;
                }
            } else if (written[2] == '7') {
                switch (val) {
                    case '0':
                        bg = 8; // Dark gray
                        break;
                    case '1':
                        bg = 12; // Light Red
                        break;
                    case '2':
                        bg = 10; // Light Green
                        break;
                    case '3':
                        bg = 14; // Yellow
                        break;
                    case '4':
                        bg = 9; // Light blue
                        break;
                    case '5':
                        bg = 13; // Light Magenta
                        break;
                    case '6':
                        bg = 11; // Light Cyan
                        break;
                    case '7':
                        bg = 15; // White
                        break;
                }
            }
            written += 4;
        } else {
            size_t index           = y * VGA_WIDTH + x++;
            uint8_t color          = ((0xF & bg) << 4) | (0xF & fg);
            internal_buffer[index] = val | (color << 8);
        }
        if (x == VGA_WIDTH) {
            x = 0;
            y++;
        }
        if (y == VGA_HEIGHT) {
            for (int ny = 1; ny < VGA_HEIGHT; ny++) {
                libcxx::memcpy((void*)&(internal_buffer[(ny - 1) * VGA_WIDTH]),
                               (void*)&(internal_buffer[ny * VGA_WIDTH]),
                               2 * VGA_WIDTH);
            }
            libcxx::memset(
                (void*)&(internal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH]), 0,
                2 * VGA_WIDTH);
            y = VGA_HEIGHT - 1;
        }
    }
    if (this->id == current_tty) {
        libcxx::memcpy(vga_buffer, internal_buffer, sizeof(internal_buffer));
        update_cursor(x, y);
    }
    return count;
}

bool vtty::handle_kb(input::dtk_event_type type, unsigned code, int value)
{
    if (current_tty == this->id && type == input::dtk_event_type::key) {
        if (value != 0) {
            switch (code) {
                case KEY_LEFTCTRL:
                case KEY_RIGHTCTRL:
                    state.ctrl = true;
                    return true;
                case KEY_LEFTALT:
                case KEY_RIGHTALT:
                    state.alt = true;
                    return true;
                case KEY_LEFTSHIFT:
                case KEY_RIGHTSHIFT:
                    state.shift = true;
                    return true;
                case KEY_LEFTMETA:
                case KEY_RIGHTMETA:
                    log::printk(log::log_level::INFO, "Meta key");
                    state.meta = true;
                    return true;
            }
        } else {
            switch (code) {
                case KEY_LEFTCTRL:
                case KEY_RIGHTCTRL:
                    state.ctrl = false;
                    break;
                case KEY_LEFTALT:
                case KEY_RIGHTALT:
                    state.alt = false;
                    break;
                case KEY_LEFTSHIFT:
                case KEY_RIGHTSHIFT:
                    state.shift = false;
                    break;
                case KEY_LEFTMETA:
                case KEY_RIGHTMETA:
                    state.meta = false;
                    break;
            }
            return true;
        }
        uint16_t* key_map = base_map;
        // TODO: Caps lock?
        if (state.shift == true) {
            key_map = shift_map;
        }
        if (code < NUM_CODES) {
            uint16_t key = key_map[code];
            uint8_t type = (key >> 8) & 0xFF;
            uint8_t val  = key & 0xFF;
            switch (type) {
                case 0xF1:
                    // FN keys
                    if (state.ctrl && state.alt) {
                        switch_vtty(val + 1);
                    }
                    break;
                case 0xF2:
                    if (val == 0x01) {
                        val = '\r';
                    }
                    this->handle_input(&val, 1);
                    break;
                default:
                    this->handle_input(&val, 1);
            }
            return true;
        }
    }
    return false;
}

void vtty::init_termios(struct termios& termios)
{
    termios.c_iflag  = ICRNL | IXON;
    termios.c_oflag  = OPOST | ONLCR;
    termios.c_cflag  = B38400 | CS8 | CREAD | HUPCL;
    termios.c_lflag  = ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN;
    termios.c_ispeed = termios.c_ospeed = 38400;
    libcxx::memcpy(termios.c_cc, init_cc, num_init_cc);
}

int vtty::get_id()
{
    return this->id;
}

void vtty::restore()
{
    libcxx::memcpy(vga_buffer, internal_buffer, sizeof(internal_buffer));
    update_cursor(x, y);
}

void switch_vtty(int next)
{
    if (next < 1 || next > NUM_VTTYS) {
        return;
    }
    if (next == current_tty) {
        return;
    }
    current_tty = next;
    vttys[current_tty]->restore();
}

void vtty_init()
{
    addr_t virt = memory::vmalloc::allocate(VGA_BUFFER_SIZE);
    if (!memory::virt::map_range(virt, VGA_BUFFER_BASE, VGA_BUFFER_SIZE,
                                 PAGE_WRITABLE)) {
        log::printk(log::log_level::WARNING, "Failed to map VGA buffer\n");
        return;
    }
    vga_buffer = reinterpret_cast<uint16_t*>(virt);

    enable_cursor();
    register_class(device_class::CHR, vtty_major);

    for (int i = 1; i <= NUM_VTTYS; i++) {
        vtty* v  = new vtty(i);
        vttys[i] = v;
        register_kdevice(device_class::CHR, vtty_major, i,
                         register_tty(v, vtty_major, i));
    }
    current_tty = 1;
}
} // namespace terminal
} // namespace filesystem
