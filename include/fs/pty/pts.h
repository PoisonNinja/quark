#pragma once

#include <fs/driver.h>
#include <fs/inode.h>
#include <fs/pty/pty.h>
#include <fs/tmpfs/tmpfs.h>

namespace Filesystem
{
class PTSFS : public TmpFS
{
public:
    PTSFS();
    // Returns minor
    bool register_pty(TTY::PTY* pty);
    bool mount(Superblock* sb) override;

private:
    libcxx::list<TTY::PTY, &TTY::PTY::node> ptys;
    InitFS::Directory* root;
};

} // namespace Filesystem
