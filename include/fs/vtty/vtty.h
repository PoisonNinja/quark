#pragma once

#include <drivers/input/input.h>
#include <fs/tty.h>

namespace filesystem
{
namespace terminal
{
struct kb_state {
    kb_state()
        : ctrl(false)
        , shift(false)
        , alt(false)
        , meta(false){};
    bool ctrl, shift, alt, meta;
    unsigned last;
};

class vtty : public tty_driver
{
public:
    vtty(int id);
    int ioctl(unsigned long request, char* argp) override;
    int open(const char* name) override;
    ssize_t write(const uint8_t* buffer, size_t count) override;
    void init_termios(struct termios& termios) override;

    // VTTY specific functions
    bool handle_kb(input::dtk_event_type type, unsigned code, int value);
    int get_id();

    // Called when this vtty is selected
    void restore();

private:
    struct kb_state state;
    struct input::input_handler* handler;
    int id;
    int x, y;
    int bg, fg;
    uint16_t internal_buffer[80 * 25];
};

void switch_vtty(int next);
void vtty_init();
} // namespace terminal
} // namespace filesystem
