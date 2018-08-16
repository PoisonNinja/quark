#include <arch/drivers/io.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/pci_list.h>
#include <kernel.h>

namespace PCI
{
namespace
{
List<Driver, &Driver::node> drivers;
List<Device, &Device::node> devices;

Driver* match_device(Device& device)
{
    PCIID id = device.get_pciid();
    if (drivers.empty()) {
        return nullptr;
    }
    for (auto& driver : drivers) {
        if (driver.filter.vendor_id == id.vendor_id &&
            driver.filter.device_id == id.device_id) {
            Log::printk(Log::LogLevel::INFO,
                        "pci: Found driver %s for device %X:%X\n",
                        driver.name(), id.vendor_id, id.device_id);
            return &driver;
        }
    }
    Log::printk(Log::LogLevel::WARNING,
                "pci: Didn't find driver for %X:%X based on fine search, "
                "switching to coarse\n",
                id.vendor_id, id.device_id);
    for (auto& driver : drivers) {
        if (driver.filter.class_id == id.class_id &&
            driver.filter.subclass_id == id.subclass_id) {
            Log::printk(
                Log::LogLevel::INFO,
                "pci: Found driver %s for device %X:%X by coarse search\n",
                driver.name(), id.vendor_id, id.device_id);
            return &driver;
        }
    }
    Log::printk(Log::LogLevel::ERROR,
                "pci: Didn't find driver for %X:%X at all\n", id.vendor_id,
                id.device_id);
    return nullptr;
}

void match_all_devices()
{
    for (auto& device : devices) {
        if (!device.is_claimed()) {
            Driver* driver = match_device(device);
            if (driver) {
                driver->probe(&device);
            }
        }
    }
}

constexpr uint16_t config_address = 0xCF8;
constexpr uint16_t config_data    = 0xCFC;

uint32_t read_register(const uint16_t bus, const uint16_t dev,
                       const uint16_t func, const uint32_t reg)
{
    outl(config_address, 0x80000000L | (static_cast<uint32_t>(bus) << 16) |
                             (static_cast<uint32_t>(dev) << 11) |
                             (static_cast<uint32_t>(func) << 8) | (reg & ~3));
    return inl(config_data);
}

void write_register(const uint16_t bus, const uint16_t dev, const uint16_t func,
                    const uint32_t reg, const uint32_t data)
{
    outl(config_address, 0x80000000L | (static_cast<uint32_t>(bus) << 16) |
                             (static_cast<uint32_t>(dev) << 11) |
                             (static_cast<uint32_t>(func) << 8) | (reg & ~3));
    outl(config_data, data);
}

uint32_t read_32(const uint16_t bus, const uint16_t dev, const uint16_t func,
                 const uint8_t offset)
{
    /*
     * Registers are aligned on a 4-byte boundary. Therefore, we can basically
     * round down to the register by clearing the lower two bits.
     */
    return read_register(bus, dev, func, offset & ~3U);
}

void write_32(const uint16_t bus, const uint16_t dev, const uint16_t func,
              const uint8_t offset, const uint32_t data)
{
    return write_register(bus, dev, func, offset & ~3U, data);
}

uint16_t read_16(const uint16_t bus, const uint16_t dev, const uint16_t func,
                 const uint8_t offset)
{
    uint32_t val = read_register(bus, dev, func, offset & ~3U);
    /*
     * We could use unions, but that is undefined behavior in C++
     * (type-punning). Even though GCC apparently guarantees that this is valid
     * even with strict aliasing as a GNU extension, it's better not to rely on
     * this so we could port more easily to other compilers (e.g. Clang).
     */
    uint32_t shift = (offset & 3) * 8; // Amount to shift
    uint16_t ret   = (val >> shift) & 0xFFFF;
    return ret;
}

void write_16(const uint16_t bus, const uint16_t dev, const uint16_t func,
              const uint8_t offset, const uint16_t data)
{
    // Get current value (we're only modifying some bits)
    uint32_t val   = read_register(bus, dev, func, offset & ~3U);
    uint32_t shift = (offset & 3) * 8; // Amount to shift
    uint32_t mask  = ~(0xFFFF << shift);
    val &= mask;
    val |= (data << shift);
    return write_register(bus, dev, func, offset & ~3U, val);
}

uint8_t read_8(const uint16_t bus, const uint16_t dev, const uint16_t func,
               const uint8_t offset)
{
    uint32_t val   = read_register(bus, dev, func, offset & ~3U);
    uint32_t shift = (offset & 3) * 8; // Amount to shift
    uint8_t ret    = (val >> shift) & 0xFF;
    return ret;
}

void write_8(const uint16_t bus, const uint16_t dev, const uint16_t func,
             const uint8_t offset, const uint8_t data)
{
    // Get current value (we're only modifying some bits)
    uint32_t val   = read_register(bus, dev, func, offset & ~3U);
    uint32_t shift = (offset & 3) * 8; // Amount to shift
    uint32_t mask  = ~(0xFF << shift);
    val &= mask;
    val |= (data << shift);
    return write_register(bus, dev, func, offset & ~3U, val);
}

void probe_bus(uint8_t bus);

void probe_device(uint8_t bus, uint8_t dev)
{
    // 0xFFFF for vendor indicates a not-present device
    if (read_16(bus, dev, 0, pci_vendor_id) == 0xFFFF)
        return;
    int max_functions = 1;
    // Leading bit of type indicates multifunction
    if (!(read_8(bus, dev, 0, pci_type) & 0x80)) {
        max_functions = 8;
    }
    for (int i = 0; i < max_functions; i++) {
        uint16_t vendor_id = read_16(bus, dev, i, pci_vendor_id);
        if (vendor_id != 0xFFFF) {
            uint8_t class_id    = read_8(bus, dev, i, pci_class);
            uint8_t subclass_id = read_8(bus, dev, i, pci_subclass);

            // Check if this is a PCI-PCI bus. If it is, we need to scan that
            if (class_id == 0x6 && subclass_id == 0x4) {
                uint8_t next_bus = read_8(bus, dev, i, pci_secondary_bus);
                probe_bus(next_bus);
            }

            uint16_t device_id      = read_16(bus, dev, i, pci_device_id);
            PCI_VENTABLE pci_vendor = {
                .VenShort = "Unknown",
                .VenFull  = "Unknown",
            };
            for (uint16_t i = 0; i < PCI_VENTABLE_LEN; i++) {
                if (PciVenTable[i].VenId == vendor_id)
                    pci_vendor = PciVenTable[i];
            }
            PCI_DEVTABLE pci_device = {
                .Chip     = "Unknown",
                .ChipDesc = "Unknown device",
            };
            for (uint16_t i = 0; i < PCI_DEVTABLE_LEN; i++) {
                if (PciDevTable[i].VenId == vendor_id) {
                    if (PciDevTable[i].DevId == device_id)
                        pci_device = PciDevTable[i];
                }
            }
            Log::printk(Log::LogLevel::INFO,
                        "pci: Found device: %X:%X (%s %s)\n", vendor_id,
                        device_id, pci_vendor.VenFull, pci_device.ChipDesc);
            Device* device = new Device(bus, dev, i);
            devices.push_back(*device);
            match_all_devices();
        }
    }
}

void probe_bus(uint8_t bus)
{
    for (int i = 0; i < 32; i++) {
        probe_device(bus, i);
    }
}

// Initial probing stuff
void probe()
{
    // Get the root PCI controller type
    uint8_t type = read_8(0, 0, 0, pci_type);
    /*
     * Check if it's multifunction by checking the leading bit of the type. If
     * it's not set, then we only have one PCI controller. Otherwise, we have
     * multiple PCI controllers on different function numbers and we need to
     * scan through them too.
     */
    if (!(type & 0x80)) {
        Log::printk(Log::LogLevel::INFO, "pci: Only one PCI host controller\n");
        probe_bus(0);
    } else {
        Log::printk(Log::LogLevel::INFO,
                    "pci: Multiple PCI host controllers\n");
        for (int i = 0; i < 8; i++) {
            if (read_16(0, 0, i, pci_vendor_id) != 0xFFFF)
                break;
            probe_bus(i);
        }
    }
}

List<Driver, &Driver::node> drivers;
} // namespace

bool register_driver(Driver& d)
{
    drivers.push_back(d);
    return true;
}

void init()
{
    Log::printk(Log::LogLevel::INFO, "pci: Initializing...\n");
    probe();
}

Device::Device(uint8_t bus, uint8_t device, uint8_t function)
    : bus(bus)
    , device(device)
    , function(function)
{
}

uint32_t Device::read_config_32(const uint8_t offset)
{
    return read_32(this->bus, this->device, this->function, offset);
}

void Device::write_config_32(const uint8_t offset, const uint32_t value)
{
    write_32(this->bus, this->device, this->function, offset, value);
}

uint16_t Device::read_config_16(const uint8_t offset)
{
    return read_16(this->bus, this->device, this->function, offset);
}

void Device::write_config_16(const uint8_t offset, const uint16_t value)
{
    write_16(this->bus, this->device, this->function, offset, value);
}

uint8_t Device::read_config_8(const uint8_t offset)
{
    return read_8(this->bus, this->device, this->function, offset);
}

void Device::write_config_8(const uint8_t offset, const uint8_t value)
{
    write_8(this->bus, this->device, this->function, offset, value);
}
} // namespace PCI