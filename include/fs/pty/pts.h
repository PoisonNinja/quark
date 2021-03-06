#pragma once

#include <fs/driver.h>
#include <fs/inode.h>
#include <fs/tmpfs/tmpfs.h>
#include <fs/tty.h>

namespace filesystem
{
namespace terminal
{
class ptm;

class pts : public tty_driver
{
public:
    pts(tty* master);
    int open(const char* name) override;
    ssize_t write(const uint8_t* buffer, size_t count) override;
    void init_termios(struct termios& termios) override;

private:
    tty* master;
};
} // namespace terminal

/*
 * ptsfs is really an instance of tmpfs, just hacked to register things
 * automatically
 */
class ptsfs : public tmpfs::driver
{
public:
    ptsfs();
    int register_ptm(terminal::tty* ptm);
    bool mount(superblock* sb) override;

private:
    tmpfs::directory* root;
    int index;
};
} // namespace filesystem
