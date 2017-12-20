#include <boot/info.h>
#include <kernel.h>
#include <kernel/version.h>
#include <mm/mm.h>

void kmain(struct Boot::info& info)
{
    Log::printk(Log::INFO, "%s\n", OS_STRING);
    Log::printk(Log::INFO, "Command line: %s\n", info.cmdline);
    Memory::init(info);
}
