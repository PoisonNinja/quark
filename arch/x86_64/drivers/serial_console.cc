#include <arch/drivers/io.h>
#include <arch/drivers/serial_console.h>

// Serial port
#define SERIAL_PORT 0x3F8

size_t X86Serial::write(const char *message, size_t size)
{
    size_t written = 0;
    while (size--) {
        outb(SERIAL_PORT, message[written++]);
    }
    return written;
}
