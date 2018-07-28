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
    // Register the filesystem drivers
    FTable::add("tmpfs", new TmpFS());
    FTable::add("pts", new PTSFS());

    VGATTY* vga = new VGATTY();
    reserve_class(CHR, 0);
    register_kdevice(CHR, 0, vga);

    // Ref<Inode> tty(new VGATTY());
    // Ref<Vnode> vtty(new Vnode(tty));
    // Ref<Descriptor> dtty(new Descriptor(vtty));
    // Ref<Inode> ptmx(new PTMX(0, 0, 0755));
    // Ref<Vnode> vptmx(new Vnode(ptmx));
    // Ref<Descriptor> dptmx(new Descriptor(vptmx));

    // Initialize the root filesystem
    Superblock* rootsb = new Superblock();
    FTable::get("tmpfs")->mount(rootsb);

    Ref<Vnode> vroot(new Vnode(rootsb, rootsb->root));
    Ref<Descriptor> droot(new Descriptor(vroot));
    vroot->link(".", vroot);
    vroot->link("..", vroot);

    droot->mkdir("dev", 0666);
    droot->mkdir("tmp", 0666);
    // droot->link("dev/tty", dtty);

    // Initialize the PTY subsystem
    // droot->link("dev/ptmx", dptmx);
    droot->mkdir("dev/pts", 0666);
    droot->mount("pts", "dev/pts", "pts", 0);

    Scheduler::get_current_process()->set_cwd(droot);
    Scheduler::get_current_process()->set_root(droot);
}
}  // namespace Filesystem
