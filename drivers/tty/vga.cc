#include <arch/drivers/io.h>
#include <drivers/tty/vga.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/valloc.h>
#include <mm/virtual.h>

namespace
{
const addr_t VGA_BUFFER_BASE = 0xB8000;
const size_t VGA_BUFFER_SIZE = 0x8000;
const int VGA_HEIGHT         = 25;
const int VGA_WIDTH          = 80;
} // namespace

namespace Filesystem
{
namespace TTY
{

VGATTY::VGATTY()
    : KDevice(CHR)
{
    addr_t virt = Memory::Valloc::allocate(VGA_BUFFER_SIZE);
    if (!Memory::Virtual::map_range(virt, VGA_BUFFER_BASE, VGA_BUFFER_SIZE,
                                    PAGE_WRITABLE | PAGE_HARDWARE)) {
        Log::printk(Log::LogLevel::WARNING, "Failed to map VGA buffer\n");
        return;
    }
    vga_buffer = reinterpret_cast<uint16_t *>(virt);
}

VGATTY::~VGATTY()
{
    Memory::Valloc::free(reinterpret_cast<addr_t>(vga_buffer));
}

ssize_t VGATTY::write(uint8_t *buffer, size_t size, off_t offset,
                      void * /* cookie */)
{
    libcxx::memcpy((void *)((uint8_t *)vga_buffer + offset), buffer, size);
    return size;
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
} // namespace TTY
} // namespace Filesystem
