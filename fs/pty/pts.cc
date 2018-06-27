#include <fs/pty/pts.h>
#include <lib/list.h>

namespace Filesystem
{
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

PTSD::PTSD(ino_t ino, dev_t dev, mode_t mode)
{
    this->ino = (ino) ? ino : reinterpret_cast<ino_t>(this);
    this->dev = (dev) ? dev : reinterpret_cast<dev_t>(this);
    this->mode = mode;
}

PTSD::~PTSD()
{
}

int PTSD::link(const char* name, Ref<Inode> node)
{
}

Ref<Inode> PTSD::open(const char* name, int flags, mode_t mode)
{
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
}  // namespace Filesystem