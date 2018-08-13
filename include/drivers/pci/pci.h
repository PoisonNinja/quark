#pragma once

#include <lib/list.h>
#include <types.h>

namespace PCI
{
class Device
{
public:
    Node<Device> node;

private:
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t subclass_id;
    uint8_t class_id;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};

/*
 * For now, a driver can either support one specific model or one entire family
 * of devices. In the future we may look into multiple vendors/devices, but this
 * will suffice for now.
 */
struct Filter {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_id;
    uint8_t subclass_id;
};

class Driver
{
public:
    virtual bool probe(Device* dev) = 0;
    virtual const char* name() = 0;
    Filter filter;
    Node<Driver> node;
};

bool register_driver(Driver* d);

void init();
}  // namespace PCI