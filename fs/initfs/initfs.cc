#include <fs/fs.h>
#include <fs/initfs/initfs.h>
#include <kernel.h>
#include <lib/string.h>

namespace Filesystem
{
namespace InitFS
{
InitFSNode::InitFSNode(Ref<Inode> inode, const char* name)
{
    this->inode = inode;
    this->name = String::strdup(name);
}

InitFSNode::~InitFSNode()
{
    delete[] this->name;
}

File::File(ino_t ino, dev_t dev, mode_t mode)
{
    this->ino = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->dev = (dev) ? dev : reinterpret_cast<dev_t>(this);
    this->mode = mode;
    // Allocate when actually used
    data = nullptr;
    buffer_size = 0;
}

File::~File()
{
}

ssize_t File::pread(uint8_t* buffer, size_t count, off_t offset)
{
    // Trim out of bound reads
    if (count + offset > buffer_size) {
        count = buffer_size - offset;
    }
    String::memcpy(buffer, data + offset, count);
    return count;
}

ssize_t File::pwrite(uint8_t* buffer, size_t count, off_t offset)
{
    if (count + offset > buffer_size) {
        size_t new_buffer_size = count + offset;
        uint8_t* new_buffer = new uint8_t[new_buffer_size];
        String::memcpy(new_buffer, data, buffer_size);
        buffer_size = new_buffer_size;
        data = new_buffer;
    }
    String::memcpy(data + offset, buffer, count);
    return count;
}

Directory::Directory(ino_t ino, dev_t dev, mode_t mode)
{
    this->ino = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->dev = (dev) ? dev : reinterpret_cast<dev_t>(this);
    this->mode = mode;
}

Directory::~Directory()
{
    if (!children.empty()) {
        Log::printk(Log::WARNING,
                    "initfs directory freed, but children not freed.\n");
    }
}

Ref<Inode> Directory::open(const char* name, int flags, mode_t mode)
{
    Ref<Inode> ret = find_child(name);
    if (ret) {
        return ret;
    } else if (!ret && !(flags & O_CREAT)) {
        return Ref<Inode>(nullptr);
    }
    Ref<File> child(new File(0, 0, mode));
    InitFSNode* node = new InitFSNode(child, name);
    children.push_back(*node);
    return child;
}

Ref<Inode> Directory::find_child(const char* name)
{
    for (auto& i : children) {
        if (!String::strcmp(i.name, name)) {
            return i.inode;
        }
    }
    return Ref<Inode>(nullptr);
}
}
}
