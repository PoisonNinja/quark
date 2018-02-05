#include <boot/info.h>
#include <cpu/interrupt.h>
#include <kernel.h>
#include <kernel/time/time.h>
#include <kernel/version.h>
#include <mm/mm.h>
#include <proc/sched.h>

void kmain(struct Boot::info& info)
{
    Log::printk(Log::INFO, "%s\n", OS_STRING);
    Log::printk(Log::INFO, "Command line: %s\n", info.cmdline);
    Memory::init(info);
    Interrupt::init();
    Time::init();
    Interrupt::enable();
    Scheduler::init();
    for (;;)
        __asm__("hlt");
}
