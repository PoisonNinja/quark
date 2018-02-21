#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/initfs/initfs.h>
#include <fs/inode.h>
#include <fs/vnode.h>
#include <proc/sched.h>

namespace Filesystem
{
void init()
{
    Ref<Inode> iroot(new InitFS::Directory(0, 0, 0755));
    Ref<Vnode> vroot(new Vnode(iroot));
    Ref<Descriptor> droot(new Descriptor(vroot));
    Scheduler::get_current_process()->cwd = droot;
    Scheduler::get_current_process()->root = droot;
}
}
