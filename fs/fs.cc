#include <drivers/tty/vga.h>
#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/ftable.h>
#include <fs/inode.h>
#include <fs/pty/ptm.h>
#include <fs/pty/pts.h>
#include <fs/tmpfs/tmpfs.h>
#include <fs/tty.h>
#include <fs/vnode.h>
#include <kernel.h>
#include <proc/sched.h>

namespace Filesystem
{
void init()
{
    FTable::add("tmpfs", new TmpFS());
    FTable::add("pts", new PTSFS());

    Superblock* rootsb = new Superblock();
    FTable::get("tmpfs")->mount(rootsb);

    Ref<Inode> iroot(new InitFS::Directory(0, 0, 0755));
    Ref<Vnode> vroot(new Vnode(rootsb, iroot));
    Ref<Descriptor> droot(new Descriptor(vroot));

    // Ref<Inode> tty(new VGATTY());
    // Ref<Vnode> vtty(new Vnode(tty));
    // Ref<Descriptor> dtty(new Descriptor(vtty));
    // Ref<Inode> ptmx(new PTMX(0, 0, 0755));
    // Ref<Vnode> vptmx(new Vnode(ptmx));
    // Ref<Descriptor> dptmx(new Descriptor(vptmx));

    iroot->link(".", iroot);
    iroot->link("..", iroot);
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
