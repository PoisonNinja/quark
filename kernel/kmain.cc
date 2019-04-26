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
        log::printk(log::log_level::ERROR, "Failed to open init\n");
        for (;;)
            cpu::halt();
    }
    struct filesystem::stat st;
    init->stat(&st);
    log::printk(log::log_level::DEBUG, "init binary has size of %zu bytes\n",
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
    struct thread_context ctx;
    if (!scheduler::get_current_thread()->load(
            reinterpret_cast<addr_t>(init_raw), argc, argv, envc, envp, ctx)) {
        log::printk(log::log_level::ERROR, "Failed to load thread state\n");
    } else {
        log::printk(log::log_level::DEBUG,
                    "Preparing to jump into userspace\n");
    }
    delete[] init_raw;
    load_registers(ctx);
}

void kidle_trampoline(void*)
{
    return scheduler::idle();
}

void init_stage1()
{
    addr_t cloned  = memory::virt::fork();
    process* initp = new process(nullptr);
    scheduler::add_process(initp);
    initp->set_root(scheduler::get_current_process()->get_root());
    initp->set_cwd(scheduler::get_current_process()->get_cwd());
    initp->address_space = cloned;

    thread* stage2 = create_kernel_thread(initp, init_stage2, nullptr);
    scheduler::insert(stage2);
    create_kernel_thread(scheduler::get_current_process(), kidle_trampoline,
                         nullptr);
    initp->wait(0);
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
