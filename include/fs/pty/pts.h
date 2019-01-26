#pragma once

#include <fs/driver.h>
#include <fs/inode.h>
#include <fs/pty/pty.h>
#include <fs/tmpfs/tmpfs.h>

namespace filesystem
{
class ptsfs : public tmpfs::driver
{
public:
    ptsfs();
    // Returns minor
    bool register_pty(tty::pty* pty);
    bool mount(superblock* sb) override;

private:
    libcxx::list<tty::pty, &tty::pty::node> ptys;
    tmpfs::directory* root;
};

} // namespace filesystem
