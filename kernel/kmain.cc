#include <boot/info.h>
#include <cpu/cpu.h>
#include <cpu/interrupt.h>
#include <drivers/pci/pci.h>
#include <fs/fs.h>
#include <fs/initrd/initrd.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/time/time.h>
#include <kernel/version.h>
#include <lib/string.h>
#include <mm/mm.h>
#include <mm/virtual.h>
#include <proc/sched.h>
#include <proc/syscall.h>

void init_stage2(void*)
{
    Process* parent = Scheduler::get_current_process();
    std::shared_ptr<Filesystem::Descriptor> root = parent->get_root();
    std::shared_ptr<Filesystem::Descriptor> init =
        root->open("/sbin/init", O_RDONLY, 0);
    if (!init) {
        Log::printk(Log::LogLevel::ERROR, "Failed to open init\n");
        for (;;)
            CPU::halt();
    }
    struct Filesystem::stat st;
    init->stat(&st);
    Log::printk(Log::LogLevel::DEBUG, "init binary has size of %zu bytes\n",
                st.st_size);
    uint8_t* init_raw = new uint8_t[st.st_size];
    init->read(init_raw, st.st_size);
    int argc           = 2;
    const char* argv[] = {
        "/sbin/init",
        "test",
    };
    int envc           = 1;
    const char* envp[] = {
        "hello=world",
    };
    struct ThreadContext ctx;
    if (!Scheduler::get_current_thread()->load(
            reinterpret_cast<addr_t>(init_raw), argc, argv, envc, envp, ctx)) {
        Log::printk(Log::LogLevel::ERROR, "Failed to load thread state\n");
    } else {
        Log::printk(Log::LogLevel::DEBUG, "Preparing to jump into userspace\n");
    }
    delete[] init_raw;
    load_registers(ctx);
}

void init_stage1()
{
    addr_t cloned  = Memory::Virtual::fork();
    Process* initp = new Process(nullptr);
    Scheduler::add_process(initp);
    initp->set_root(Scheduler::get_current_process()->get_root());
    initp->set_cwd(Scheduler::get_current_process()->get_cwd());
    initp->set_dtable(
        std::shared_ptr<Filesystem::DTable>(new Filesystem::DTable));
    initp->address_space = cloned;

    Thread* stage2 = create_kernel_thread(initp, init_stage2, nullptr);
    Scheduler::insert(stage2);
    Scheduler::idle();
}

void kmain(struct Boot::info& info)
{
    Log::printk(Log::LogLevel::INFO, "%s\n", OS_STRING);
    Log::printk(Log::LogLevel::INFO, "Command line: %s\n", info.cmdline);
    Memory::init(info);
    Interrupt::init();
    Interrupt::enable();
    Time::init();
    Scheduler::init();
    Signal::init();
    Filesystem::init();
    Filesystem::Initrd::init(info);
    Syscall::init();

    PCI::init();

    /*
     * Core subsystems are online, let's starting bringing up the rest of
     * the kernel.
     */
    do_initcall(InitLevel::EARLY);
    do_initcall(InitLevel::CORE);
    do_initcall(InitLevel::ARCH);
    do_initcall(InitLevel::SUBSYS);
    do_initcall(InitLevel::FS);
    do_initcall(InitLevel::DEVICE);
    do_initcall(InitLevel::LATE);

    init_stage1();
}
