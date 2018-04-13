#include <boot/info.h>
#include <cpu/interrupt.h>
#include <fs/fs.h>
#include <fs/initrd/initrd.h>
#include <fs/stat.h>
#include <kernel.h>
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
    Ref<Filesystem::Descriptor> root = parent->get_root();
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
    int argc = 2;
    const char* argv[] = {
        "/sbin/init",
        "test",
    };
    int envc = 1;
    const char* envp[] = {
        "hello=world",
    };
    struct ThreadContext ctx;
    if (!Scheduler::get_current_thread()->load(
            reinterpret_cast<addr_t>(init_raw), argc, argv, envc, envp, ctx)) {
        Log::printk(Log::ERROR, "Failed to load thread state\n");
    } else {
        Log::printk(Log::DEBUG, "Preparing to jump into userspace\n");
    }
    delete[] init_raw;
    load_registers(ctx);
}

void init_stage1()
{
    addr_t cloned = Memory::Virtual::fork();
    Process* initp = new Process(nullptr);
    initp->set_root(Scheduler::get_current_process()->get_root());
    initp->set_cwd(Scheduler::get_current_process()->get_cwd());
    initp->set_dtable(Ref<Filesystem::DTable>(new Filesystem::DTable));
    initp->address_space = cloned;

    Thread* stage2 = create_kernel_thread(initp, init_stage2, nullptr);
    Scheduler::insert(stage2);
    Scheduler::idle();
}

void kmain(struct Boot::info& info)
{
    Log::printk(Log::INFO, "%s\n", OS_STRING);
    Log::printk(Log::INFO, "Command line: %s\n", info.cmdline);
    Memory::init(info);
    Interrupt::init();
    Interrupt::enable();
    Time::init();
    Scheduler::init();
    Filesystem::init();
    Filesystem::Initrd::init(info);
    Syscall::init();
    init_stage1();
}
