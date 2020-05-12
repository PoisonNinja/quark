#include <arch/drivers/io.h>
#include <cpu/interrupt.h>
#include <drivers/input/codes.h>
#include <drivers/input/input.h>
#include <fs/dev.h>
#include <kernel.h>
#include <kernel/init.h>
#include <lib/functional.h>
#include <proc/wq.h>

using namespace libcxx::placeholders;
using namespace input;

namespace
{
constexpr size_t buffer_size = 1024;

// clang-format off
int scancode_table[] = {
    [0x00] = KEY_INVALID,
    [0x01] = KEY_ESC,
    [0x02] = KEY_1,
    [0x03] = KEY_2,
    [0x04] = KEY_3,
    [0x05] = KEY_4,
    [0x06] = KEY_5,
    [0x07] = KEY_6,
    [0x08] = KEY_7,
    [0x09] = KEY_8,
    [0x0a] = KEY_9,
    [0x0b] = KEY_0,
    [0x0c] = KEY_MINUS,
    [0x0d] = KEY_EQUAL,
    [0x0e] = KEY_BACKSPACE,
    [0x0f] = KEY_TAB,
    [0x10] = KEY_Q,
    [0x11] = KEY_W,
    [0x12] = KEY_E,
    [0x13] = KEY_R,
    [0x14] = KEY_T,
    [0x15] = KEY_Y,
    [0x16] = KEY_U,
    [0x17] = KEY_I,
    [0x18] = KEY_O,
    [0x19] = KEY_P,
    [0x1a] = KEY_LEFTBRACE,
    [0x1b] = KEY_RIGHTBRACE,
    [0x1c] = KEY_ENTER,
    [0x1d] = KEY_LEFTCTRL,
    [0x1e] = KEY_A,
    [0x1f] = KEY_S,
    [0x20] = KEY_D,
    [0x21] = KEY_F,
    [0x22] = KEY_G,
    [0x23] = KEY_H,
    [0x24] = KEY_J,
    [0x25] = KEY_K,
    [0x26] = KEY_L,
    [0x27] = KEY_SEMICOLON,
    [0x28] = KEY_APOSTROPHE,
    [0x29] = KEY_GRAVE,
    [0x2a] = KEY_LEFTSHIFT,
    [0x2b] = KEY_BACKSLASH,
    [0x2c] = KEY_Z,
    [0x2d] = KEY_X,
    [0x2e] = KEY_C,
    [0x2f] = KEY_V,
    [0x30] = KEY_B,
    [0x31] = KEY_N,
    [0x32] = KEY_M,
    [0x33] = KEY_COMMA,
    [0x34] = KEY_DOT,
    [0x35] = KEY_SLASH,
    [0x36] = KEY_RIGHTSHIFT,
    [0x37] = KEY_KPASTERISK,
    [0x38] = KEY_LEFTALT,
    [0x39] = KEY_SPACE,
    [0x3A] = KEY_CAPSLOCK,
    [0x3b] = KEY_F1,
    [0x3c] = KEY_F2,
    [0x3d] = KEY_F3,
    [0x3e] = KEY_F4,
    [0x3f] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,
    [0x45] = KEY_NUMLOCK,
    [0x46] = KEY_SCROLLLOCK,
    [0x47] = KEY_HOME,
    [0x48] = KEY_UP,
    [0x49] = KEY_PAGEUP,
    [0x4a] = KEY_KPMINUS,
    [0x4b] = KEY_LEFT,
    [0x4c] = KEY_KP5,
    [0x4d] = KEY_RIGHT,
    [0x4e] = KEY_KPPLUS,
    [0x4f] = KEY_END,
    [0x50] = KEY_DOWN,
    [0x51] = KEY_PAGEDOWN,
    [0x52] = KEY_INSERT,
    [0x53] = KEY_DELETE,
};
// clang-format on

class intel8042 : public input::device
{
public:
    intel8042();

private:
    static unsigned scancode_to_inputcode(unsigned scancode);

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
    this->handle_event(input::dtk_event_type::key,
                       scancode_to_inputcode(scancode), is_press);
}

unsigned intel8042::scancode_to_inputcode(unsigned scancode)
{
    if (scancode == 0xE0)
        kernel::panic("No 0xE0 scancode support!");
    if (scancode >= sizeof(scancode_table))
        kernel::panic("Unsupported scancode");
    return scancode_table[scancode];
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
