#include <fs/fs.h>
#include <fs/initrd/initrd.h>
#include <fs/initrd/tar.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/valloc.h>
#include <mm/virtual.h>
#include <proc/sched.h>

namespace
{
constexpr const char* tar_magic = "ustar";
}

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

bool parse(addr_t initrd)
{
    addr_t current = initrd;
    std::shared_ptr<Descriptor> root =
        Scheduler::get_current_process()->get_root();
    std::shared_ptr<Descriptor> file;
    struct Filesystem::Initrd::Tar::Header term;
    String::memset(&term, 0, sizeof(term));
    int terminator = 0;
    while (1) {
        struct Filesystem::Initrd::Tar::Header* header =
            reinterpret_cast<Filesystem::Initrd::Tar::Header*>(current);
        if (String::memcmp(header->magic, tar_magic, 6)) {
            if (!String::memcmp(header, &term, sizeof(*header))) {
                if (++terminator == 2) {
                    Log::printk(Log::LogLevel::INFO,
                                "initrd: Found second terminator, done!\n");
                    return true;
                }
                current += sizeof(*header);
                continue;
            }
            Log::printk(Log::LogLevel::ERROR, "initrd: Bad tar magic\n");
            return false;
        }
        size_t size = decode_octal(header->size);
        Log::printk(Log::LogLevel::INFO,
                    "initrd: Name: %s, Size: %zu, Type: %u\n", header->name,
                    size, header->typeflag);
        switch (header->typeflag) {
            case '0':
                file = root->open(header->name, O_CREAT | O_RDWR, 0755);
                if (!file) {
                    Log::printk(Log::LogLevel::ERROR,
                                "initrd: Failed to open file\n");
                    break;
                }
                file->write(reinterpret_cast<uint8_t*>(current + 512), size);
                break;
            case '5':
                root->mkdir(header->name, 0755);
                break;
            default:
                Log::printk(Log::LogLevel::WARNING,
                            "initrd: Got unknown file type, skipping\n");
        }
        current += 512 + ((decode_octal(header->size) + 512 - 1) / 512) * 512;
    }
    return true;
}

void init(struct Boot::info& info)
{
    size_t size = info.initrd_end - info.initrd_start;
    Log::printk(Log::LogLevel::INFO,
                "initrd: Initrd located at %p - %p, size %p\n",
                info.initrd_start, info.initrd_end, size);
    addr_t virt = Memory::Valloc::allocate(size);
    if (!Memory::Virtual::map_range(virt, info.initrd_start, size,
                                    PAGE_WRITABLE)) {
        Log::printk(Log::LogLevel::ERROR,
                    "initrd: Failed to map initrd into memory\n");
        return;
    }
    if (!parse(virt)) {
        Log::printk(Log::LogLevel::ERROR, "initrd: Failed to parse initrd\n");
    } else {
        Log::printk(Log::LogLevel::INFO, "initrd: Initrd loaded\n");
    }
    Memory::Virtual::unmap_range(virt, size);
}
} // namespace Initrd
} // namespace Filesystem
