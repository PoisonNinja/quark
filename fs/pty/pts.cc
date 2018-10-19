#include <errno.h>
#include <fs/pty/pts.h>
#include <kernel.h>
#include <lib/list.h>
#include <lib/string.h>

namespace Filesystem
{
PTSN::PTSN(ino_t ino, dev_t dev, mode_t mode)
{
    this->ino  = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->mode = mode;
    this->size = 0;
    this->uid  = 0;
    this->gid  = 0;
}

PTSN::~PTSN()
{
}

ssize_t PTSN::read(uint8_t* buffer, size_t count, off_t offset)
{
}

ssize_t PTSN::write(uint8_t* buffer, size_t count, off_t offset)
{
}

PTSNWrapper::PTSNWrapper(Ref<Inode> inode, const char* name)
{
    this->inode = inode;
    this->name  = String::strdup(name);
}

PTSNWrapper::~PTSNWrapper()
{
    delete[] this->name;
}

PTSD::PTSD(ino_t ino, dev_t dev, mode_t mode)
{
    this->ino  = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->mode = mode;
}

PTSD::~PTSD()
{
}

int PTSD::link(const char* name, Ref<Inode> node)
{
    Ref<Inode> child = find_child(name);
    if (child) {
        return -EEXIST;
    }
    PTSNWrapper* wrapper = new PTSNWrapper(node, name);
    children.push_back(*wrapper);
    return 0;
}

Ref<Inode> PTSD::lookup(const char* name, int flags, mode_t mode)
{
    Ref<Inode> ret = find_child(name);
    if (ret) {
        return ret;
    } else if (!ret && !(flags & O_CREAT)) {
        return Ref<Inode>(nullptr);
    }
    Ref<PTSN> child(new PTSN(0, 0, mode));
    PTSNWrapper* node = new PTSNWrapper(child, name);
    children.push_back(*node);
    return child;
}

Ref<Inode> PTSD::find_child(const char* name)
{
    for (auto& i : children) {
        if (!String::strcmp(i.name, name)) {
            return i.inode;
        }
    }
    return Ref<Inode>(nullptr);
}

PTSFS::PTSFS()
{
}

PTSFS::~PTSFS()
{
}

bool PTSFS::mount(Superblock* sb)
{
    sb->root = Ref<Inode>(new PTSD(0, 0, 0755));
    return true;
}

uint32_t PTSFS::flags()
{
    return driver_pseudo;
}
} // namespace Filesystem
