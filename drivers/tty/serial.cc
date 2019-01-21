#include <arch/drivers/io.h>
#include <drivers/tty/serial.h>

// Serial port
constexpr uint16_t serial_port = 0x3F8;

size_t serial::write(const char *message, size_t size)
{
    size_t written = 0;
    while (size--) {
        outb(serial_port, message[written++]);
    }
    return written;
}
