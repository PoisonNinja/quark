#pragma once

#include <fs/dev.h>
#include <lib/list.h>

namespace input
{
constexpr size_t buffer_size = 1024;

// Modelled after Linux EV_* events
// Device To Kernel events
enum class dtk_event_type {
    key,
    rel,
    abs,
};

// Kernel To Device events
enum class ktd_event_type {
    led,
    snd,
};

struct event_data {
    unsigned type;
    unsigned code;
    int value;
};

class input_kdevice : public filesystem::kdevice
{
public:
    input_kdevice();
    virtual int poll(filesystem::poll_register_func_t& callback) override;
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    virtual bool seekable() override;

    void append(unsigned type, unsigned code, int val);

private:
    event_data buffer[buffer_size];
    size_t head, tail;
    scheduler::wait_queue queue;
};

class device
{
public:
    device();
    virtual int event(ktd_event_type type, unsigned code, int value);

    void set_kdevice(input_kdevice*);
    input_kdevice* get_kdevice();

private:
    input_kdevice* kdev;
};

bool register_device(device* dev);
void report_event(device* dev, dtk_event_type type, unsigned code, int value);
} // namespace input
