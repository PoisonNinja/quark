#pragma once

#include <fs/driver.h>
#include <fs/inode.h>

namespace Filesystem
{
class PTSFS : public Driver
{
public:
    PTSFS();
    ~PTSFS();

    bool mount(Superblock* sb) override;
    uint32_t flags() override;
};

class PTSN : public BaseInode
{
public:
    PTSN(ino_t ino, dev_t dev, mode_t mode);
    virtual ~PTSN();
    virtual ssize_t pread(uint8_t* buffer, size_t count, off_t offset) override;
    virtual ssize_t pwrite(uint8_t* buffer, size_t count,
                           off_t offset) override;
};

struct PTSNWrapper {
    PTSNWrapper(Ref<Inode> inode, const char* name);
    ~PTSNWrapper();
    Ref<Inode> inode;
    const char* name;
    Node<PTSNWrapper> node;
};

class PTSD : public BaseInode
{
public:
    PTSD(ino_t ino, dev_t dev, mode_t mode);
    virtual ~PTSD();
    virtual int link(const char* name, Ref<Inode> node) override;
    virtual Ref<Inode> open(const char* name, int flags, mode_t mode) override;

private:
    Ref<Inode> find_child(const char* name);
    List<PTSNWrapper, &PTSNWrapper::node> children;
};
}  // namespace Filesystem