#pragma once

#include <fs/dev.h>
#include <fs/inode.h>

namespace filesystem
{
namespace tty
{
class tty
{
public:
    tty();
    ~tty();

    virtual libcxx::pair<int, void*> open(const char* name);
    virtual int ioctl(unsigned long request, char* argp, void* cookie);

    virtual int poll(filesystem::poll_register_func_t& callback, void* cookie);
    virtual ssize_t read(uint8_t* buffer, size_t count, void* cookie);
    virtual ssize_t write(const uint8_t* buffer, size_t count, void* cookie);
};

class tty_device : public filesystem::kdevice
{
public:
    tty_device(tty* driver);

    int ioctl(unsigned long request, char* argp, void* cookie) override;

    libcxx::pair<int, void*> open(const char* name) override;

    int poll(filesystem::poll_register_func_t& callback, void* cookie);
    ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                 void* cookie) override;
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                  void* cookie) override;

private:
    tty* t;
};

class tty_core;

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

/*
 * To facilitate ptmx, per-terminal data is stored in the tty_driver
 * itself.
 */

struct termios {
};

class tty_driver
{
public:
    tty_driver();
    virtual ssize_t write(const uint8_t* buffer, size_t count, void* cookie);
    void set_core(tty_core* core);
    struct termios* get_termios();

private:
    tty_core* core;
};

class tty_core : public kdevice
{
public:
    tty_core(tty_driver* driver);
    int ioctl(unsigned long request, char* argp, void* cookie) override final;
    libcxx::pair<int, void*> open(const char* name) override;
    int poll(filesystem::poll_register_func_t& callback, void* cookie) override;
    ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                 void* cookie) override;
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                  void* cookie) override;

public:
    ssize_t notify(const uint8_t* buffer, size_t count);

private:
    tty_driver* driver;

    char* buffer;
};

bool register_tty(dev_t major, tty* tty);

bool register_tty_ng(tty_driver* driver, dev_t major, dev_t minor);

void init();
} // namespace tty
} // namespace filesystem
