#include <drivers/fb/vga.h>
#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/ftable.h>
#include <fs/inode.h>
#include <fs/pty/ptm.h>
#include <fs/pty/pts.h>
#include <fs/stat.h>
#include <fs/tmpfs/tmpfs.h>
#include <fs/tty.h>
#include <fs/vnode.h>
#include <kernel.h>
#include <proc/sched.h>

namespace Filesystem
{
void init()
{
    // Initialize the TTY layer
    TTY::init();

    // Register the filesystem drivers
    Drivers::add("tmpfs", new TmpFS());
    Drivers::add("ptsfs", new PTSFS());

    VGAFB* vga     = new VGAFB();
    auto vga_major = Filesystem::locate_class(Filesystem::CHR);
    register_class(CHR, vga_major);
    register_kdevice(CHR, vga_major, vga);

    // Initialize the root filesystem
    Superblock* rootsb = new Superblock();
    Drivers::get("tmpfs")->mount(rootsb);

    libcxx::intrusive_ptr<Vnode> vroot(new Vnode(rootsb, rootsb->root));
    libcxx::intrusive_ptr<Descriptor> droot(
        new Descriptor(vroot, F_READ | F_WRITE));
    vroot->link(".", vroot);
    vroot->link("..", vroot);

    droot->mkdir("dev", 0666);
    droot->mkdir("tmp", 0666);
    droot->mkdir("dev/pts", 0666);
    droot->mount("", "dev/pts", "ptsfs", 0);

    Scheduler::get_current_process()->set_cwd(droot);
    Scheduler::get_current_process()->set_root(droot);
}
} // namespace Filesystem
