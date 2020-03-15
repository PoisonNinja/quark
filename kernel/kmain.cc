#include <boot/info.h>
#include <cpu/cpu.h>
#include <cpu/interrupt.h>
#include <drivers/pci/pci.h>
#include <fs/fs.h>
#include <fs/initrd/initrd.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/symbol.h>
#include <kernel/time/time.h>
#include <kernel/version.h>
#include <lib/string.h>
#include <mm/mm.h>
#include <mm/virtual.h>
#include <proc/sched.h>
#include <proc/syscall.h>

namespace
{
void init_stage2(void*)
{
    process* parent = scheduler::get_current_process();
    libcxx::intrusive_ptr<filesystem::descriptor> root = parent->get_root();
    auto [err, init] = root->open("/sbin/init", O_RDONLY, 0);
    if (err) {
        kernel::panic("Failed to open init\n");
    }
    int argc           = 2;
    const char* argv[] = {
        "/sbin/init",
        "test",
    };
    int envc           = 1;
    const char* envp[] = {
        "hello=world",
    };
    struct thread_context ctx;
    if (scheduler::get_current_process()->load("/sbin/init", init, argc, argv,
                                               envc, envp, ctx)) {
        kernel::panic("Failed to load init\n");
    } else {
        log::printk(log::log_level::DEBUG,
                    "Preparing to jump into userspace\n");
    }
    load_registers(ctx);
}

void init_stage1()
{
    process* initp = scheduler::get_current_process()->fork();
    thread* stage2 = create_thread(initp, init_stage2, nullptr);
    scheduler::late_init();
    scheduler::insert(stage2);
    scheduler::get_current_process()->wait(-1, nullptr, 0);
    kernel::panic("init exited!\n");
}
} // namespace

void kmain(struct boot::info& info)
{
    log::printk(log::log_level::INFO, "%s\n", OS_STRING);
    log::printk(log::log_level::INFO, "Command line: %s\n", info.cmdline);
    memory::init(info);
    interrupt::init();
    interrupt::enable();
    symbols::init();
    time::init();
    scheduler::init();
    signal::init();
    filesystem::init();
    filesystem::initrd::init(info);
    syscall::init();

    pci::init();

    /*
     * Core subsystems are online, let's starting bringing up the rest of
     * the kernel.
     */
    do_initcall(init_level::EARLY);
    do_initcall(init_level::CORE);
    do_initcall(init_level::ARCH);
    do_initcall(init_level::SUBSYS);
    do_initcall(init_level::FS);
    do_initcall(init_level::DEVICE);
    do_initcall(init_level::LATE);

    init_stage1();
}
