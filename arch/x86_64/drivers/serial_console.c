#include <arch/drivers/io.h>
#include <kernel/console.h>

// Serial port
#define SERIAL_PORT 0x3F8

static size_t x86_64_console_serial_write(const char *message, size_t size)
{
    size_t written = 0;
    while (size--) {
        outb(SERIAL_PORT, message[written++]);
    }
    return written;
}

static struct console x86_64_console_serial = {
    .write = x86_64_console_serial_write,
    .name = "x86_64 serial",
};

status_t x86_64_initialize_serial(void)
{
    console_register(&x86_64_console_serial);
    return SUCCESS;
}
