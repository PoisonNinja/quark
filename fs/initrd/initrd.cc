#include <fs/fs.h>
#include <fs/initrd/initrd.h>
#include <fs/initrd/tar.h>
#include <kernel.h>
#include <mm/valloc.h>
#include <mm/virtual.h>
#include <proc/sched.h>

namespace Filesystem
{
namespace Initrd
{
size_t decode_octal(char size[12])
{
    size_t total = 0;
    for (int i = 0; i < 11; i++) {
        total *= 8;
        total += static_cast<int>(size[i]) - '0';
    }
    return total;
}

void parse(addr_t initrd)
{
    addr_t current = initrd;
    Ref<Descriptor> root = Scheduler::get_current_process()->get_root();
    Ref<Descriptor> file;
    while (1) {
        struct Filesystem::Initrd::Tar::Header* header =
            reinterpret_cast<Filesystem::Initrd::Tar::Header*>(current);
        if (header->magic[0] == 0) {
            return;
        }
        size_t size = decode_octal(header->size);
        Log::printk(Log::INFO, "Name: %s, Size: %llu, Type: %u\n", header->name,
                    size, header->typeflag);
        switch (header->typeflag) {
            case '0':
                file = root->open(header->name, O_CREAT | O_RDWR, 0755);
                if (!file) {
                    Log::printk(Log::ERROR, "Failed to write open file\n");
                    break;
                }
                file->write(reinterpret_cast<uint8_t*>(current + 512), size);
                break;
            case '5':
                root->mkdir(header->name, 0755);
                break;
        }
        current += 512 + ((decode_octal(header->size) + 512 - 1) / 512) * 512;
    }
}

void init(struct Boot::info& info)
{
    size_t size = info.initrd_end - info.initrd_start;
    Log::printk(Log::INFO, "Initrd located at %p - %p, size %llX\n",
                info.initrd_start, info.initrd_end, size);
    addr_t virt = Memory::Valloc::allocate(size);
    if (!Memory::Virtual::map_range(virt, info.initrd_start, size,
                                    PAGE_WRITABLE)) {
        Log::printk(Log::ERROR, "Failed to map initrd into memory\n");
        return;
    }
    parse(virt);
    Log::printk(Log::INFO, "Initrd loaded\n");
}
}
}
