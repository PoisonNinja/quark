#pragma once

#include <fs/inode.h>
#include <lib/list.h>

namespace Filesystem
{
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

private:
    uint8_t* buffer;
    size_t buffer_size;
    size_t buffer_used;
};

class Directory : public BaseInode
{
public:
    Directory(ino_t ino, dev_t dev, mode_t mode);
    virtual ~Directory();
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode);

private:
    Ref<Inode> find_child(const char* name);
    List<InitFSNode, &InitFSNode::node> children;
};
}
}
