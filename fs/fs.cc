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
    dev_t major = locate_class(CHR);
    register_class(CHR, major);
    register_kdevice(CHR, major, vga);

    // Initialize the root filesystem
    Superblock* rootsb = new Superblock();
    FTable::get("tmpfs")->mount(rootsb);

    Ref<Vnode> vroot(new Vnode(rootsb, rootsb->root));
    Ref<Descriptor> droot(new Descriptor(vroot));
    vroot->link(".", vroot);
    vroot->link("..", vroot);

    droot->mkdir("dev", 0666);
    droot->mkdir("tmp", 0666);

    Scheduler::get_current_process()->set_cwd(droot);
    Scheduler::get_current_process()->set_root(droot);
}
} // namespace Filesystem
