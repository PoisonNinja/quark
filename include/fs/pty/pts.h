#pragma once

#include <fs/driver.h>
#include <fs/inode.h>
#include <fs/tmpfs/tmpfs.h>
#include <fs/tty.h>

namespace filesystem
{
namespace tty
{
class ptm;

class pts : public tty_driver
{
public:
    pts(ptm* master);
    libcxx::pair<int, void*> open(const char* name) override;
    ssize_t write(const uint8_t* buffer, size_t count) override;

    ssize_t notify(const uint8_t* buffer, size_t count);

    void init_termios(struct termios& termios) override;

private:
    ptm* master;
};
} // namespace tty

/*
 * ptsfs is really an instance of tmpfs, just hacked to register things
 * automatically
 */
class ptsfs : public tmpfs::driver
{
public:
    ptsfs();
    int register_ptm(tty::ptm* ptm);
    bool mount(superblock* sb) override;

private:
    tmpfs::directory* root;
    int index;
};
} // namespace filesystem
