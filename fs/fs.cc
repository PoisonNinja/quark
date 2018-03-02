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
    Scheduler::get_current_process()->cwd = droot;
    Scheduler::get_current_process()->root = droot;
    Log::printk(Log::INFO, "%d\n", droot->mkdir("/sbin", 0755));
    Ref<Descriptor> test = droot->open("/sbin/test", O_CREAT, 0755);
    if (!test) {
        Log::printk(Log::ERROR, "Error opening /sbin/test\n");
    } else {
        Log::printk(Log::INFO, "Opened /sbin/test successfully!\n");
        test->write((uint8_t*)"Hello", 6);
    }
    Ref<Descriptor> test2 = droot->open("/sbin/test", 0, 0);
    if (!test2) {
        Log::printk(Log::ERROR, "Error opening /sbin/test2\n");
    } else {
        Log::printk(Log::INFO, "Opened /sbin/test2 successfully!\n");
        uint8_t buffer[6];
        test2->read(buffer, 6);
        if (String::strcmp("Hello", (const char*)buffer)) {
            buffer[5] = '\0';
            Log::printk(Log::ERROR, "Data read back is not what we wrote: %s\n",
                        buffer);
        } else {
            Log::printk(Log::INFO, "Data read back IS what we wrote!\n");
        }
    }
}
}
