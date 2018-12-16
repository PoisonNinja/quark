#pragma once

#include <fs/inode.h>
#include <fs/tty.h>

namespace Filesystem
{
namespace TTY
{
class PTMX : public TTY
{
public:
    PTMX();
    virtual ~PTMX();

    virtual libcxx::pair<int, void*> open(const char* name) override;
    virtual int ioctl(unsigned long request, char* argp, void* cookie) override;

    virtual ssize_t read(uint8_t* buffer, size_t count, void* cookie) override;
    virtual ssize_t write(uint8_t* buffer, size_t count, void* cookie) override;

private:
    size_t next_pty_number;
};

} // namespace TTY
} // namespace Filesystem
