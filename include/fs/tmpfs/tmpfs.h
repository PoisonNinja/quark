#pragma once

#include <fs/driver.h>
#include <fs/inode.h>
#include <lib/list.h>

namespace filesystem
{
namespace tmpfs
{
class driver : public filesystem::driver
{
public:
    driver();
    ~driver();
    bool mount(superblock* sb) override;
    uint32_t flags() override;
};

struct tmpfs_node {
    tmpfs_node(libcxx::intrusive_ptr<inode> inode, const char* name);
    tmpfs_node(const struct tmpfs_node& other);
    tmpfs_node& operator=(const struct tmpfs_node& other);
    ~tmpfs_node();
    libcxx::intrusive_ptr<inode> ino;
    const char* name;
    libcxx::node<tmpfs_node> node;
};

class file : public inode
{
public:
    file(ino_t ino, dev_t rdev, mode_t mode);
    virtual ~file();
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                         void* cookie) override;
    virtual ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                          void* cookie) override;

private:
    uint8_t* data;
    size_t buffer_size;
};

class directory : public inode
{
public:
    directory(ino_t ino, dev_t rdev, mode_t mode);
    virtual ~directory();
    virtual int link(const char* name,
                     libcxx::intrusive_ptr<inode> node) override;
    virtual libcxx::intrusive_ptr<inode> lookup(const char* name, int flags,
                                                mode_t mode) override;
    virtual int mkdir(const char* name, mode_t mode) override;
    virtual int mknod(const char* name, mode_t mode, dev_t dev) override;

private:
    libcxx::intrusive_ptr<inode> find_child(const char* name);
    libcxx::list<tmpfs_node, &tmpfs_node::node> children;
};
} // namespace tmpfs
} // namespace filesystem
