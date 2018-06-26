#include <drivers/tty/vga.h>
#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/ftable.h>
#include <fs/inode.h>
#include <fs/pty.h>
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
    Ref<Inode> iroot(new InitFS::Directory(0, 0, 0755));
    Ref<Vnode> vroot(new Vnode(iroot));
    Ref<Descriptor> droot(new Descriptor(vroot));
    Ref<Inode> tty(new VGATTY());
    Ref<Vnode> vtty(new Vnode(tty));
    Ref<Descriptor> dtty(new Descriptor(vtty));
    iroot->link(".", iroot);
    iroot->link("..", iroot);
    droot->mkdir("dev", 0666);
    droot->mkdir("tmp", 0666);
    droot->link("dev/tty", dtty);

    // droot->link("dev/tty1", tty);
    Scheduler::get_current_process()->set_cwd(droot);
    Scheduler::get_current_process()->set_root(droot);
}
}  // namespace Filesystem
