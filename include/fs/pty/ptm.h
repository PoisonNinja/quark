#pragma once

#include <fs/inode.h>
#include <fs/tty.h>

namespace filesystem
{
namespace tty
{
class ptmx : public tty
{
public:
    ptmx();
    virtual ~ptmx();

    virtual libcxx::pair<int, void*> open(const char* name) override;
    virtual int ioctl(unsigned long request, char* argp, void* cookie) override;

    virtual int poll(filesystem::poll_register_func_t& callback,
                     void* cookie) override;
    virtual ssize_t read(uint8_t* buffer, size_t count, void* cookie) override;
    virtual ssize_t write(const uint8_t* buffer, size_t count,
                          void* cookie) override;

private:
    size_t next_pty_number;
};

} // namespace tty
} // namespace filesystem
