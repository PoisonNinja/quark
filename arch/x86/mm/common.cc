#include <arch/kernel/multiboot2.h>
#include <arch/mm/layout.h>
#include <arch/mm/mm.h>
#include <boot/info.h>
#include <kernel.h>
#include <mm/virtual.h>

namespace Memory
{
namespace X86
{
bool is_valid_physical_memory(addr_t m, struct Boot::info &info)
{
    struct multiboot_fixed *multiboot =
        reinterpret_cast<struct multiboot_fixed *>(info.architecture_data);
    addr_t multiboot_start =
        Memory::Virtual::align_down(reinterpret_cast<addr_t>(multiboot) - VMA);
    addr_t multiboot_end = Memory::Virtual::align_up(
        reinterpret_cast<addr_t>(multiboot) - VMA + multiboot->total_size);
    if (m >= Memory::Virtual::align_down(info.kernel_start) &&
        m < Memory::Virtual::align_up(info.kernel_end)) {
        Log::printk(Log::LogLevel::DEBUG,
                    "        Rejected %p because in kernel\n", m);
        return false;
    } else if (m >= multiboot_start && m < multiboot_end) {
        Log::printk(Log::LogLevel::DEBUG,
                    "        Rejected %p because in multiboot\n", m);
        return false;
    } else if (m >= Memory::Virtual::align_down(info.initrd_start) &&
               m < Memory::Virtual::align_up(info.initrd_end)) {
        Log::printk(Log::LogLevel::DEBUG,
                    "        Rejected %p because in initrd\n", m);
        return false;
    }
    return true;
}
} // namespace X86
} // namespace Memory