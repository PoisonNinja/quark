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
const char* dirname(const char* original)
{
    char* path = String::strdup(original);
    char* p;
    if (path == NULL || *path == '\0')
        return "/";
    p = path + String::strlen(path) - 1;
    while (*p == '/') {
        if (p == path)
            return path;
        *p-- = '\0';
    }
    while (p >= path && *p != '/')
        p--;
    return p < path ? "/" : p == path ? "/" : (*p = '\0', path);
}

static char* basename(const char* name)
{
    const char* base = name;

    while (*name) {
        if (*name++ == '/') {
            base = name;
        }
    }
    return (char*)base;
}

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
            Filesystem::InodeCache::set(next);
        }
        inode = next;
    }
    delete[] str;
    return inode;
}

static Inode* resolve_real_create(Inode* start, const char* path, int flags,
                                  mode_t mode)
{
    Inode* ret = nullptr;
    const char* dir = dirname(path);
    const char* base = basename(path);
    Inode* parent = resolve_real(start, dir, (flags & ~O_CREAT));
    if (!parent) {
        delete[] dir;
        return nullptr;
    }
    if ((ret = resolve_real(parent, base, flags))) {
        return ret;
    }
    Dentry* dentry = new Dentry;
    String::strncpy(dentry->name, base, DENTRY_MAX_LENGTH);
    ret = parent->create(dentry, flags, mode);
    Filesystem::InodeCache::set(ret);
    delete dentry;
    delete[] dir;
    return ret;
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
    if (flags & O_CREAT) {
        return resolve_real_create(start, path, flags, mode);
    } else {
        return resolve_real(start, path, flags);
    }
}
}
}
