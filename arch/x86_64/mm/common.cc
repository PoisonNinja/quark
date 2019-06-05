#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <arch/mm/mm.h>
#include <boot/info.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace memory
{
namespace x86_64
{
bool is_valid_physical_memory(addr_t m, struct boot::info &info,
                              addr_t multiboot_start, addr_t multiboot_end)
{
    if (m >= memory::virt::align_down(info.kernel_start) &&
        m < memory::virt::align_up(info.kernel_end)) {
        log::printk(log::log_level::DEBUG,
                    "        Rejected %p because in kernel\n", m);
        return false;
    } else if (m >= multiboot_start && m < multiboot_end) {
        log::printk(log::log_level::DEBUG,
                    "        Rejected %p because in multiboot\n", m);
        return false;
    } else if (m >= memory::virt::align_down(info.initrd_start) &&
               m < memory::virt::align_up(info.initrd_end)) {
        log::printk(log::log_level::DEBUG,
                    "        Rejected %p because in initrd\n", m);
        return false;
    }
    return true;
}
} // namespace x86_64
} // namespace memory
