#include <fs/dentry.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <lib/string.h>
#include <proc/sched.h>

#include <kernel.h>

namespace Filesystem
{
namespace Path
{
static Inode* resolve_real(Inode* start, const char* path, int flags)
{
    if (!start) {
        Log::printk(Log::WARNING, "Starting inode is invalid!\n");
        return nullptr;
    }
    Inode* inode = start;
    char* str = String::strdup(path);
    char* name;
    while (inode && (name = String::strtok_r(str, "/", &str))) {
        Log::printk(Log::DEBUG, "%s\n", name);
        Dentry* dentry = new Dentry;
        String::strncpy(dentry->name, name, DENTRY_MAX_LENGTH);
        if (inode->lookup(dentry) < 0) {
            return nullptr;
        }
        Inode* next =
            Filesystem::InodeCache::get(dentry->superblock, dentry->ino);
        if (!next) {
            next = inode->superblock->alloc_inode();
            next->ino = dentry->ino;
            next->superblock = dentry->superblock;
            next->superblock->read_inode(next);
        }
        inode = next;
    }
    delete[] str;
    return inode;
}

Inode* resolve(const char* path, int flags, mode_t mode)
{
    Inode* start = nullptr;
    if (*path == '/') {
        if (*(path + 1) == '\0') {
            return Scheduler::get_current_process()->root;
        }
        start = Scheduler::get_current_process()->root;
        path++;
    } else {
        start = Scheduler::get_current_process()->cwd;
    }
    // TODO: Handle O_CREAT
    return resolve_real(start, path, flags);
}
}
}
