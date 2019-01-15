#include <errno.h>
#include <fs/fs.h>
#include <fs/stat.h>
#include <fs/tmpfs/tmpfs.h>
#include <kernel.h>
#include <lib/string.h>

namespace filesystem
{
TmpFS::TmpFS()
{
}

TmpFS::~TmpFS()
{
}

bool TmpFS::mount(Superblock* sb)
{
    sb->root = libcxx::intrusive_ptr<Inode>(new InitFS::Directory(0, 0, 0755));
    return true;
}

uint32_t TmpFS::flags()
{
    return driver_pseudo;
}

namespace InitFS
{
TmpFSNode::TmpFSNode(libcxx::intrusive_ptr<Inode> inode, const char* name)
{
    this->inode = inode;
    this->name  = libcxx::strdup(name);
}

TmpFSNode::TmpFSNode(const struct TmpFSNode& other)
{
    this->inode = other.inode;
    this->name  = libcxx::strdup(other.name);
}

TmpFSNode& TmpFSNode::operator=(const struct TmpFSNode& other)
{
    this->inode         = other.inode;
    const char* newname = libcxx::strdup(other.name);
    libcxx::swap(this->name, newname);
    return *this;
}

TmpFSNode::~TmpFSNode()
{
    delete[] this->name;
}

File::File(ino_t ino, dev_t rdev, mode_t mode)
{
    this->ino  = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->rdev = rdev;
    this->mode = mode;
    this->size = 0;
    this->uid  = 0;
    this->gid  = 0;
    // Allocate when actually used
    data        = nullptr;
    buffer_size = 0;
}

File::~File()
{
    if (this->data) {
        delete[] data;
    }
}

ssize_t File::read(uint8_t* buffer, size_t count, off_t offset, void* cookie)
{
    // Trim out of bound reads
    if (count + offset > buffer_size) {
        count = buffer_size - offset;
    }
    libcxx::memcpy(buffer, data + offset, count);
    return count;
}

ssize_t File::write(const uint8_t* buffer, size_t count, off_t offset,
                    void* cookie)
{
    if (count + offset > buffer_size) {
        size_t new_buffer_size = count + offset;
        uint8_t* new_buffer    = new uint8_t[new_buffer_size];
        if (data) {
            libcxx::memcpy(new_buffer, data, buffer_size);
            delete[] data;
        }
        buffer_size = new_buffer_size;
        data        = new_buffer;
    }
    this->size = buffer_size;
    libcxx::memcpy(data + offset, buffer, count);
    return count;
}

Directory::Directory(ino_t ino, dev_t rdev, mode_t mode)
{
    this->ino  = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->rdev = rdev;
    this->mode = mode | S_IFDIR;
}

Directory::~Directory()
{
    if (!children.empty()) {
        Log::printk(Log::LogLevel::WARNING,
                    "initfs directory freed, but children not freed.\n");
    }
}

int Directory::link(const char* name, libcxx::intrusive_ptr<Inode> node)
{
    libcxx::intrusive_ptr<Inode> child = find_child(name);
    if (child) {
        return -EEXIST;
    }
    children.push_back(*(new TmpFSNode(node, name)));
    return 0;
}

libcxx::intrusive_ptr<Inode> Directory::lookup(const char* name, int flags,
                                               mode_t mode)
{
    libcxx::intrusive_ptr<Inode> ret = find_child(name);
    if (ret == nullptr) {
    }
    if (ret) {
        return ret;
    } else if (!ret && !(flags & O_CREAT)) {
        return libcxx::intrusive_ptr<Inode>(nullptr);
    }
    libcxx::intrusive_ptr<File> child(new File(0, 0, mode));
    TmpFSNode* node = new TmpFSNode(child, name);
    children.push_back(*node);
    return child;
}

int Directory::mkdir(const char* name, mode_t mode)
{
    libcxx::intrusive_ptr<Inode> child = find_child(name);
    if (child) {
        return -EEXIST;
    }
    libcxx::intrusive_ptr<Directory> dir(new Directory(0, 0, mode));
    dir->link(".", dir);
    dir->link("..", libcxx::intrusive_ptr<Directory>(this));
    TmpFSNode* node = new TmpFSNode(dir, name);
    children.push_back(*node);
    return 0;
}

int Directory::mknod(const char* name, mode_t mode, dev_t dev)
{
    libcxx::intrusive_ptr<File> child(new File(0, dev, mode));
    TmpFSNode* node = new TmpFSNode(child, name);
    children.push_back(*node);
    return 0;
}

libcxx::intrusive_ptr<Inode> Directory::find_child(const char* name)
{
    for (auto& i : children) {
        if (!libcxx::strcmp(i.name, name)) {
            return i.inode;
        }
    }
    return libcxx::intrusive_ptr<Inode>(nullptr);
}
} // namespace InitFS
} // namespace filesystem
