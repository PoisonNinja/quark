#pragma once

#include <fs/driver.h>
#include <fs/inode.h>
#include <list>

namespace Filesystem
{
class TmpFS : public Driver
{
public:
    TmpFS();
    ~TmpFS();
    bool mount(Superblock* sb) override;
    uint32_t flags() override;
};

namespace InitFS
{
struct TmpFSNode {
    TmpFSNode(std::shared_ptr<Inode> inode, const char* name);
    TmpFSNode(const struct TmpFSNode& other);
    TmpFSNode& operator=(const struct TmpFSNode& other);
    ~TmpFSNode();
    std::shared_ptr<Inode> inode;
    const char* name;
};

class File : public BaseInode
{
public:
    File(ino_t ino, dev_t rdev, mode_t mode);
    virtual ~File();
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                         void* cookie) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, off_t offset,
                          void* cookie) override;

private:
    uint8_t* data;
    size_t buffer_size;
};

class Directory : public BaseInode
{
public:
    Directory(ino_t ino, dev_t rdev, mode_t mode);
    virtual ~Directory();
    virtual int link(const char* name, std::shared_ptr<Inode> node) override;
    virtual std::shared_ptr<Inode> lookup(const char* name, int flags,
                                          mode_t mode) override;
    virtual int mkdir(const char* name, mode_t mode) override;
    virtual int mknod(const char* name, mode_t mode, dev_t dev) override;

private:
    std::shared_ptr<Inode> find_child(const char* name);
    std::list<TmpFSNode> children;
};
} // namespace InitFS
} // namespace Filesystem
