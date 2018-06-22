#pragma once

#include <fs/driver.h>
#include <fs/inode.h>
#include <lib/list.h>

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
struct InitFSNode {
    InitFSNode(Ref<Inode> inode, const char* name);
    ~InitFSNode();
    Ref<Inode> inode;
    char* name;
    Node<InitFSNode> node;
};

class File : public BaseInode
{
public:
    File(ino_t ino, dev_t dev, mode_t mode);
    virtual ~File();
    virtual ssize_t pread(uint8_t* buffer, size_t count, off_t offset) override;
    virtual ssize_t pwrite(uint8_t* buffer, size_t count,
                           off_t offset) override;

private:
    uint8_t* data;
    size_t buffer_size;
};

class Directory : public BaseInode
{
public:
    Directory(ino_t ino, dev_t dev, mode_t mode);
    virtual ~Directory();
    virtual int link(const char* name, Ref<Inode> node) override;
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode) override;
    virtual int mkdir(const char* name, mode_t mode) override;

private:
    Ref<Inode> find_child(const char* name);
    List<InitFSNode, &InitFSNode::node> children;
};
}  // namespace InitFS
}  // namespace Filesystem
