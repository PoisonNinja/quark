#include <drivers/tty/vga.h>
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
    FTable::add("tmpfs", new TmpFS());
    FTable::add("ptsfs", new PTSFS());

    TTY::VGATTY* vga = new TTY::VGATTY();
    TTY::register_tty(0, vga);

    // Initialize the root filesystem
    Superblock* rootsb = new Superblock();
    FTable::get("tmpfs")->mount(rootsb);

    Ref<Vnode> vroot(new Vnode(rootsb, rootsb->root));
    Ref<Descriptor> droot(new Descriptor(vroot));
    vroot->link(".", vroot);
    vroot->link("..", vroot);

    droot->mkdir("dev", 0666);
    droot->mkdir("tmp", 0666);
    auto pts = droot->mkdir("dev/pts", 0666);
    droot->mount("", "dev/pts", "ptsfs", 0);

    Scheduler::get_current_process()->set_cwd(droot);
    Scheduler::get_current_process()->set_root(droot);
}
} // namespace Filesystem
