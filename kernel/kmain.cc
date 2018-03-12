#include <boot/info.h>
#include <cpu/interrupt.h>
#include <fs/fs.h>
#include <fs/initrd/initrd.h>
#include <fs/stat.h>
#include <kernel.h>
#include <proc/syscall.h>
#include <kernel/time/time.h>
#include <kernel/version.h>
#include <lib/string.h>
#include <mm/mm.h>
#include <mm/virtual.h>
#include <proc/elf.h>
#include <proc/sched.h>

void init_go()
{
    addr_t cloned = Memory::Virtual::clone();
    Process* initp = new Process(nullptr);
    initp->root = Scheduler::get_current_process()->root;
    initp->cwd = Scheduler::get_current_process()->cwd;
    initp->address_space = cloned;
    Memory::Virtual::set_address_space_root(cloned);
    Thread* thread = new Thread(initp);
    initp->add_thread(thread);
    Ref<Filesystem::Descriptor> root = Scheduler::get_current_process()->root;
    Ref<Filesystem::Descriptor> init = root->open("/sbin/init", 0, 0);
    if (!init) {
        Log::printk(Log::ERROR, "Failed to open init\n");
        for (;;)
            __asm__("hlt");
    }
    struct Filesystem::stat st;
    init->stat(&st);
    Log::printk(Log::DEBUG, "init binary has size of %llu bytes\n", st.st_size);
    uint8_t* init_raw = new uint8_t[st.st_size];
    init->read(init_raw, st.st_size);
    ELF::load(reinterpret_cast<addr_t>(init_raw), thread);
    Log::printk(Log::DEBUG, "Preparing to jump into userspace\n");
    Scheduler::insert(thread);
    // Commit suicide
    if (!Scheduler::remove(Scheduler::get_current_thread())) {
        Log::printk(Log::WARNING, "Failed to deschedule kinit\n");
    }
    for (;;) {
        // Loop until we're descheduled
        asm("hlt");
    }
}

void kmain(struct Boot::info& info)
{
    Log::printk(Log::INFO, "%s\n", OS_STRING);
    Log::printk(Log::INFO, "Command line: %s\n", info.cmdline);
    Memory::init(info);
    Interrupt::init();
    Time::init();
    Scheduler::init();
    Interrupt::enable();
    Filesystem::init();
    Filesystem::Initrd::init(info);
    Syscall::init();
    init_go();
}
