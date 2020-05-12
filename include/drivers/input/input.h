#pragma once

#include <fs/dev.h>
#include <lib/functional.h>
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

using input_handler_t =
    libcxx::function<bool(dtk_event_type type, unsigned code, int value), 64>;

struct input_handler {
    input_handler(input_handler_t handler)
        : handler(handler){};
    input_handler_t handler;
    libcxx::node<input_handler> node;
};

class device;

class input_kdevice : public filesystem::kdevice
{
public:
    input_kdevice(device* dev);
    virtual int poll(filesystem::poll_register_func_t& callback) override;
    virtual ssize_t read(uint8_t* buffer, size_t count, off_t offset) override;
    virtual bool seekable() override;

    void input_handler(dtk_event_type type, unsigned code, int val);

private:
    event_data buffer[buffer_size];
    size_t head, tail;
    scheduler::wait_queue queue;
};

using input_event_handler_t =
    libcxx::function<void(dtk_event_type type, unsigned code, int val), 64>;

class device
{
public:
    device();
    virtual int event(ktd_event_type type, unsigned code, int value);

    void set_event_handler(input_event_handler_t handler);

protected:
    void handle_event(dtk_event_type type, unsigned code, int val);

private:
    input_event_handler_t handler;
};

bool register_handler(input_handler& handler);
bool register_device(device* dev);
void report_event(device* dev, dtk_event_type type, unsigned code, int value);
} // namespace input
