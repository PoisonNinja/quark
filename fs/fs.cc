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
    drivers::add("tmpfs", new tmpfs());
    drivers::add("ptsfs", new ptsfs());

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

    droot->mkdir("dev", 0666);
    droot->mkdir("tmp", 0666);
    droot->mkdir("dev/pts", 0666);
    droot->mount("", "dev/pts", "ptsfs", 0);

    scheduler::get_current_process()->set_cwd(droot);
    scheduler::get_current_process()->set_root(droot);
}
} // namespace filesystem
