#include <fs/fs.h>
#include <fs/initrd/initrd.h>
#include <fs/initrd/tar.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/virtual.h>
#include <mm/vmalloc.h>
#include <proc/sched.h>

namespace
{
constexpr const char* tar_magic                                    = "ustar";
constexpr const struct filesystem::initrd::Tar::Header null_header = {0};
} // namespace

namespace filesystem
{
namespace initrd
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
    libcxx::intrusive_ptr<descriptor> root =
        scheduler::get_current_process()->get_root();
    int null_seen = 0;
    while (1) {
        struct filesystem::initrd::Tar::Header* header =
            reinterpret_cast<filesystem::initrd::Tar::Header*>(current);
        if (libcxx::memcmp(header->magic, tar_magic, 5)) {
            if (!libcxx::memcmp(header, &null_header, sizeof(header))) {
                if (++null_seen == 2) {
                    log::printk(log::log_level::INFO,
                                "initrd: Encountered null terminator, done!\n");
                    return true;
                }
                continue;
            } else {
                log::printk(log::log_level::ERROR, "initrd: Bad tar magic\n");
                return false;
            }
        }
        size_t size = decode_octal(header->size);
        log::printk(log::log_level::INFO,
                    "initrd: Name: %s, Size: %zu, Type: %u\n", header->name,
                    size, header->typeflag);
        switch (header->typeflag) {
            case '0': {
                auto [err, file] =
                    root->open(header->name, O_CREAT | O_RDWR, 0755);
                if (!file) {
                    log::printk(log::log_level::ERROR,
                                "initrd: Failed to open file\n");
                    break;
                }
                file->write(reinterpret_cast<uint8_t*>(current + 512), size);
                break;
            }
            case '5':
                root->mkdir(header->name, 0755);
                break;
            default:
                log::printk(log::log_level::WARNING,
                            "initrd: Got unknown file type, skipping\n");
        }
        current += 512 + ((decode_octal(header->size) + 512 - 1) / 512) * 512;
    }
    return true;
}

void init(struct boot::info& info)
{
    size_t size = info.initrd_end - info.initrd_start;
    log::printk(log::log_level::INFO,
                "initrd: Initrd located at %p - %p, size %p\n",
                info.initrd_start, info.initrd_end, size);
    addr_t virt = memory::vmalloc::allocate(size);
    if (!memory::virt::map_range(virt, info.initrd_start, size,
                                 PAGE_WRITABLE)) {
        log::printk(log::log_level::ERROR,
                    "initrd: Failed to map initrd into memory\n");
        return;
    }
    if (!parse(virt)) {
        log::printk(log::log_level::ERROR, "initrd: Failed to parse initrd\n");
    } else {
        log::printk(log::log_level::INFO, "initrd: Initrd loaded\n");
    }
    memory::virt::unmap_range(virt, size);
}
} // namespace initrd
} // namespace filesystem
