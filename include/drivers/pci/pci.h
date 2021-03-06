#pragma once

#include <lib/list.h>
#include <lib/optional.h>
#include <types.h>

namespace pci
{
/*
 * For now, a driver can either support one specific model or one entire family
 * of devices. In the future we may look into multiple vendors/devices, but this
 * will suffice for now.
 */
struct filter {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_id;
    uint8_t subclass_id;
    uint8_t prog_if;
};
#define PCI_VDEV(vendor, device) vendor, device, 0, 0, 0
#define PCI_CLASS_IF(class, subclass, prog_if) 0, 0, class, subclass, prog_if
#define PCI_CLASS(class, subclass) 0, 0, class, subclass, 0

struct pciid {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_id;
    uint8_t subclass_id;
    uint8_t prog_if;
};

struct pcibar {
    addr_t addr;
    size_t size;
};

constexpr bool bar_is_32(uint32_t low)
{
    return ((low & 0x6) == 0x0);
}

constexpr bool bar_is_64(uint32_t low)
{
    return ((low & 0x6) == 0x4);
}

class device
{
public:
    device(uint8_t bus, uint8_t device, uint8_t function);

    uint32_t read_config_32(const uint8_t offset);
    void write_config_32(const uint8_t offset, const uint32_t value);
    uint16_t read_config_16(const uint8_t offset);
    void write_config_16(const uint8_t offset, const uint16_t value);
    uint8_t read_config_8(const uint8_t offset);
    void write_config_8(const uint8_t offset, const uint8_t value);

    pciid get_pciid();
    pcibar get_pcibar(int bar);

    bool is_claimed();
    void claim();
    void unclaim();

    libcxx::node<device> node;

private:
    bool claimed = false;
    uint8_t bus;
    uint8_t dev;
    uint8_t function;
};

class driver
{
public:
    virtual ~driver(){};
    virtual bool probe(device* dev) = 0;
    virtual const char* name()      = 0;
    virtual const filter* filt()    = 0;
    libcxx::node<driver> node;
};

constexpr uint8_t pci_vendor_id = 0x00;
constexpr uint8_t pci_device_id = 0x02;
constexpr uint8_t pci_command   = 0x04;
constexpr uint8_t pci_status    = 0x06;
constexpr uint8_t pci_revision  = 0x08;
constexpr uint8_t pci_prog_if   = 0x09;
constexpr uint8_t pci_subclass  = 0x0A;
constexpr uint8_t pci_class     = 0x0B;
constexpr uint8_t pci_cache_sz  = 0x0C;
constexpr uint8_t pci_latency   = 0x0D;
constexpr uint8_t pci_type      = 0x0E;
constexpr uint8_t pci_bist      = 0x0F;

// PCI type 0/1
constexpr uint8_t pci_bar0           = 0x10;
constexpr uint8_t pci_bar1           = 0x14;
constexpr uint8_t pci_capabilities   = 0x34;
constexpr uint8_t pci_interrupt_line = 0x3C;
constexpr uint8_t pci_interrupt_pin  = 0x3D;

// PCI type 0
constexpr uint8_t pci_bar2          = 0x18;
constexpr uint8_t pci_bar3          = 0x1C;
constexpr uint8_t pci_bar4          = 0x20;
constexpr uint8_t pci_bar5          = 0x24;
constexpr uint8_t pci_cardbus       = 0x28;
constexpr uint8_t pci_subsys_vendor = 0x2C;
constexpr uint8_t pci_subsys_id     = 0x2E;
constexpr uint8_t pci_expansion_rom = 0x30;
constexpr uint8_t pci_min_grant     = 0x3E;
constexpr uint8_t pci_max_latency   = 0x3F;

// PCI type 1
constexpr uint8_t pci_primary_bus             = 0x18;
constexpr uint8_t pci_secondary_bus           = 0x19;
constexpr uint8_t pci_subordinate_bus         = 0x1A;
constexpr uint8_t pci_secondary_latency       = 0x1B;
constexpr uint8_t pci_io_base                 = 0x1C;
constexpr uint8_t pci_io_limit                = 0x1D;
constexpr uint8_t pci_secondary_status        = 0x1E;
constexpr uint8_t pci_memory_base             = 0x20;
constexpr uint8_t pci_memory_limit            = 0x22;
constexpr uint8_t pci_prefetchable_base       = 0x24;
constexpr uint8_t pci_prefetchable_limit      = 0x26;
constexpr uint8_t pci_prefetchable_base_upper = 0x28;
constexpr uint8_t pci_prefetchable_base_lower = 0x2C;
constexpr uint8_t pci_io_base_upper           = 0x30;
constexpr uint8_t pci_io_base_lower           = 0x32;
constexpr uint8_t pci_bridge_control          = 0x3E;

bool register_driver(driver& d);

libcxx::optional<addr_t> map(addr_t phys, size_t size);
void init();
} // namespace pci
