#include <errno.h>
#include <fs/fs.h>
#include <fs/stat.h>
#include <fs/tmpfs/tmpfs.h>
#include <kernel.h>
#include <lib/string.h>

namespace filesystem
{
namespace tmpfs
{
driver::driver()
{
}

driver::~driver()
{
}

bool driver::mount(superblock* sb)
{
    sb->root = libcxx::intrusive_ptr<inode>(new tmpfs::directory(0, 0, 0755));
    return true;
}

uint32_t driver::flags()
{
    return driver_pseudo;
}

tmpfs_node::tmpfs_node(libcxx::intrusive_ptr<inode> inode, const char* name)
{
    this->ino  = inode;
    this->name = libcxx::strdup(name);
}

tmpfs_node::tmpfs_node(const struct tmpfs_node& other)
{
    this->ino  = other.ino;
    this->name = libcxx::strdup(other.name);
}

tmpfs_node& tmpfs_node::operator=(const struct tmpfs_node& other)
{
    this->ino           = other.ino;
    const char* newname = libcxx::strdup(other.name);
    libcxx::swap(this->name, newname);
    return *this;
}

tmpfs_node::~tmpfs_node()
{
    delete[] this->name;
}

file::file(ino_t ino, dev_t rdev, mode_t mode)
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

file::~file()
{
    if (this->data) {
        delete[] data;
    }
}

ssize_t file::read(uint8_t* buffer, size_t count, off_t offset, void* cookie)
{
    // Trim out of bound reads
    if (count + offset > buffer_size) {
        count = buffer_size - offset;
    }
    libcxx::memcpy(buffer, data + offset, count);
    return count;
}

ssize_t file::write(const uint8_t* buffer, size_t count, off_t offset,
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

directory::directory(ino_t ino, dev_t rdev, mode_t mode)
{
    this->ino  = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->rdev = rdev;
    this->mode = mode | S_IFDIR;
}

directory::~directory()
{
    if (!children.empty()) {
        log::printk(log::log_level::WARNING,
                    "initfs directory freed, but children not freed.\n");
    }
}

int directory::link(const char* name, libcxx::intrusive_ptr<inode> node)
{
    libcxx::intrusive_ptr<inode> child = find_child(name);
    if (child) {
        return -EEXIST;
    }
    children.push_back(*(new tmpfs_node(node, name)));
    return 0;
}

libcxx::intrusive_ptr<inode> directory::lookup(const char* name, int flags,
                                               mode_t mode)
{
    libcxx::intrusive_ptr<inode> ret = find_child(name);
    if (ret == nullptr) {
    }
    if (ret) {
        return ret;
    } else if (!ret && !(flags & O_CREAT)) {
        return libcxx::intrusive_ptr<inode>(nullptr);
    }
    libcxx::intrusive_ptr<file> child(new file(0, 0, mode));
    tmpfs_node* node = new tmpfs_node(child, name);
    children.push_back(*node);
    return child;
}

int directory::mkdir(const char* name, mode_t mode)
{
    libcxx::intrusive_ptr<inode> child = find_child(name);
    if (child) {
        return -EEXIST;
    }
    libcxx::intrusive_ptr<directory> dir(new directory(0, 0, mode));
    dir->link(".", dir);
    dir->link("..", libcxx::intrusive_ptr<directory>(this));
    tmpfs_node* node = new tmpfs_node(dir, name);
    children.push_back(*node);
    return 0;
}

int directory::mknod(const char* name, mode_t mode, dev_t dev)
{
    libcxx::intrusive_ptr<file> child(new file(0, dev, mode));
    tmpfs_node* node = new tmpfs_node(child, name);
    children.push_back(*node);
    return 0;
}

libcxx::intrusive_ptr<inode> directory::find_child(const char* name)
{
    for (auto& i : children) {
        if (!libcxx::strcmp(i.name, name)) {
            return i.ino;
        }
    }
    return libcxx::intrusive_ptr<inode>(nullptr);
}
} // namespace tmpfs
} // namespace filesystem
