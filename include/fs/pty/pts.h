#pragma once

#include <fs/driver.h>
#include <fs/inode.h>
#include <fs/pty/pty.h>
#include <fs/tmpfs/tmpfs.h>

namespace filesystem
{
class ptsfs : public tmpfs
{
public:
    ptsfs();
    // Returns minor
    bool register_pty(tty::pty* pty);
    bool mount(superblock* sb) override;

private:
    libcxx::list<tty::pty, &tty::pty::node> ptys;
    InitFS::Directory* root;
};

} // namespace filesystem
