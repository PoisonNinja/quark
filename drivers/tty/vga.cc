#include <arch/drivers/io.h>
#include <drivers/tty/vga.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/valloc.h>
#include <mm/virtual.h>

namespace Filesystem
{
const addr_t VGA_BUFFER_BASE = 0xB8000;
const size_t VGA_BUFFER_SIZE = 0x8000;
const int VGA_HEIGHT = 25;
const int VGA_WIDTH = 80;

VGATTY::VGATTY()
{
    addr_t virt = Memory::Valloc::allocate(VGA_BUFFER_SIZE);
    if (!Memory::Virtual::map(virt, VGA_BUFFER_BASE, VGA_BUFFER_SIZE,
                              PAGE_WRITABLE | PAGE_HARDWARE)) {
        Log::printk(Log::WARNING, "Failed to map VGA buffer\n");
        return;
    }
    vga_buffer = reinterpret_cast<uint16_t *>(virt);
    x = y = 0;
    color = 15;
}

VGATTY::~VGATTY()
{
    Memory::Valloc::free(reinterpret_cast<addr_t>(vga_buffer));
}

ssize_t VGATTY::output(uint8_t *buffer, size_t size)
{
    size_t length = size;
    char *string = reinterpret_cast<char *>(buffer);
    while (length) {
        if (*string == '\n') {
            x = 0;
            y++;
            string++;
            length--;
        } else if (*string == '\e') {
            string += 3;
            length -= 3;
            switch (*string) {
                case '1':
                    color = 12;  // Light Red
                    break;
                case '2':
                    color = 10;  // Light Green
                    break;
                case '3':
                    color = 14;  // Yellow
                    break;
                case '6':
                    color = 11;  // Light Blue
                    break;
                case '9':
                    color = 15;  // Reset to white
                    break;
                default:
                    color = 15;
                    break;
            }
            string += 2;
            length -= 2;
        } else {
            size_t index = y * VGA_WIDTH + x++;
            vga_buffer[index] = *string++ | (color << 8);
            length--;
        }
        if (x == VGA_WIDTH) {
            x = 0;
            y++;
        }
        if (y == VGA_HEIGHT) {
            for (int ny = 1; ny < VGA_HEIGHT; ny++) {
                String::memcpy((void *)&vga_buffer[(ny - 1) * VGA_WIDTH],
                               (void *)&vga_buffer[ny * VGA_WIDTH],
                               2 * VGA_WIDTH);
            }
            String::memset((void *)&vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH], 0,
                           2 * VGA_WIDTH);
            y = VGA_HEIGHT - 1;
        }
    }
    update_cursor(x, y);
    return size - length;
}

void VGATTY::update_cursor(int col, int row)
{
    unsigned short position = (row * VGA_WIDTH) + col;

    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}
}
