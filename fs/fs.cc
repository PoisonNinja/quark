#include <drivers/fb/vga.h>
#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/pty/ptm.h>
#include <fs/pty/pts.h>
#include <fs/stat.h>
#include <fs/tmpfs/tmpfs.h>
#include <fs/tty.h>
#include <fs/vnode.h>
#include <kernel.h>
#include <proc/sched.h>

namespace filesystem
{
void init()
{
    // Initialize the TTY layer
    tty::init();

    // Register the filesystem drivers
    drivers::add("tmpfs", new tmpfs::driver());

    // Initialize the pts layer
    auto ptsfs = new filesystem::ptsfs();
    drivers::add("ptsfs", ptsfs);
    filesystem::tty::ptmx* p = new filesystem::tty::ptmx(ptsfs);
    log::printk(log::log_level::INFO, "Registering ptmx character device\n");
    filesystem::register_class(filesystem::CHR, 5);
    filesystem::register_kdevice(filesystem::CHR, 5, p);

    vgafb* vga     = new vgafb();
    auto vga_major = filesystem::locate_class(filesystem::CHR);
    register_class(CHR, vga_major);
    register_kdevice(CHR, vga_major, vga);

    // Initialize the root filesystem
    superblock* rootsb = new superblock();
    drivers::get("tmpfs")->mount(rootsb);

    libcxx::intrusive_ptr<vnode> vroot(new vnode(rootsb, rootsb->root));
    libcxx::intrusive_ptr<descriptor> droot(
        new descriptor(vroot, F_READ | F_WRITE));
    vroot->link(".", vroot);
    vroot->link("..", vroot);

    scheduler::get_current_process()->set_cwd(droot);
    scheduler::get_current_process()->set_root(droot);
}
} // namespace filesystem
