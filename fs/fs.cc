#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/initfs/initfs.h>
#include <fs/inode.h>
#include <fs/vnode.h>
#include <kernel.h>
#include <proc/sched.h>

#include <lib/string.h>

namespace Filesystem
{
void init()
{
    Ref<Inode> iroot(new InitFS::Directory(0, 0, 0755));
    Ref<Vnode> vroot(new Vnode(iroot));
    Ref<Descriptor> droot(new Descriptor(vroot));
    iroot->link(".", iroot);
    iroot->link("..", iroot);
    Scheduler::get_current_process()->cwd = droot;
    Scheduler::get_current_process()->root = droot;
}
}
