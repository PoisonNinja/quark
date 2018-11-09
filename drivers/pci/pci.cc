#include <arch/drivers/io.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/pci_list.h>
#include <kernel.h>
#include <mm/valloc.h>
#include <mm/virtual.h>

namespace PCI
{
namespace
{
libcxx::List<Driver, &Driver::node> drivers;
libcxx::List<Device, &Device::node> devices;

bool is_terminator(const Filter& filter)
{
    return !((filter.class_id) || (filter.device_id) || (filter.subclass_id) ||
             (filter.vendor_id));
}

Driver* match_device(Device& device)
{
    PCIID id = device.get_pciid();
    if (drivers.empty()) {
        return nullptr;
    }
    // We do three passes through driver lists
    // Pass 1: Match only vendor/device ID (highest granularity)
    for (auto& driver : drivers) {
        auto filter = driver.filter();
        for (size_t i = 0; !is_terminator(filter[i]); i++) {
            if (filter[i].vendor_id == id.vendor_id &&
                filter[i].device_id == id.device_id) {
                Log::printk(Log::LogLevel::INFO,
                            "pci: Found driver %s for device %X:%X "
                            "(vendor/device match)\n",
                            driver.name(), id.vendor_id, id.device_id);
                return &driver;
            }
        }
    }
    // Pass 2: Match class, subclass, and prog_if
    for (auto& driver : drivers) {
        auto filter = driver.filter();
        for (size_t i = 0; !is_terminator(filter[i]); i++) {
            if (filter[i].class_id == id.class_id &&
                filter[i].subclass_id == id.subclass_id &&
                filter[i].prog_if == id.prog_if) {
                Log::printk(Log::LogLevel::INFO,
                            "pci: Found driver %s for device %X:%X "
                            "(class/subclass/prog_if match)\n",
                            driver.name(), id.vendor_id, id.device_id);
                return &driver;
            }
        }
    }
    /*
     * Pass 3: Match class, subclass (lowest granularity). Generally speaking
     * drivers do not configure this because it has the least change of
     * actually working.
     *
     * Drivers must explicitly use PCI_CLASS instead of relying on PCI_CLASS_IF
     * to also fill out class/subclass fields
     */
    for (auto& driver : drivers) {
        auto filter = driver.filter();
        for (size_t i = 0; !is_terminator(filter[i]); i++) {
            if (filter[i].class_id == id.class_id &&
                filter[i].subclass_id == id.subclass_id && !filter[i].prog_if) {
                Log::printk(Log::LogLevel::INFO,
                            "pci: Found driver %s for device %X:%X "
                            "(class/subclass match)\n",
                            driver.name(), id.vendor_id, id.device_id);
                return &driver;
            }
        }
    }
    Log::printk(Log::LogLevel::ERROR, "pci: Didn't find driver for %X:%X\n",
                id.vendor_id, id.device_id);
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
            match_device(*device);
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
     * Check if it's multifunction by checking the leading bit of the type.
     * If it's not set, then we only have one PCI controller. Otherwise, we have
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
} // namespace

bool register_driver(Driver& d)
{
    drivers.push_back(d);
    match_all_devices();
    return true;
}

libcxx::pair<bool, addr_t> map(addr_t phys, size_t size)
{
    addr_t v = Memory::Valloc::allocate(size);
    Memory::Virtual::map_range(v, phys, size, PAGE_WRITABLE | PAGE_HARDWARE);
    return libcxx::make_pair(true, v);
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

PCIID Device::get_pciid()
{
    PCIID id;
    id.vendor_id   = this->read_config_16(pci_vendor_id);
    id.device_id   = this->read_config_16(pci_device_id);
    id.class_id    = this->read_config_8(pci_class);
    id.subclass_id = this->read_config_8(pci_subclass);
    id.prog_if     = this->read_config_8(pci_prog_if);
    return id;
}

PCIBAR Device::get_pcibar(int bar)
{
    // BAR0 = 0x10, BAR1 = 0x14, so BARX = 0x10 + (4 * x)
    uint32_t low = this->read_config_32(0x10 + (4 * bar));
    PCIBAR pcibar;
    if (bar_is_32(low)) {
        pcibar.addr = low & 0xFFFFFFF0;
        this->write_config_32(0x10 + (4 * bar), 0xFFFFFFFF);
        uint32_t size = this->read_config_32(0x10 + (4 * bar));
        pcibar.size   = ~(size & 0xFFFFFFF0) + 1;
        this->write_config_32(0x10 + (4 * bar), low);
    } else if (bar_is_64(low)) {
        uint64_t high     = this->read_config_32(0x10 + (4 * bar + 1));
        uint64_t complete = (high << 32) | low;
        pcibar.addr       = complete & 0xFFFFFFFFFFFFFFF0;
        this->write_config_32(0x10 + (4 * bar), 0xFFFFFFFF);
        this->write_config_32(0x10 + (4 * bar + 1), 0xFFFFFFFF);
        uint32_t size_low  = this->read_config_32(0x10 + (4 * bar));
        uint64_t size_high = this->read_config_32(0x10 + (4 * bar + 1));
        uint64_t size      = (size_high << 32) | size_low;
        pcibar.size        = ~(size & 0xFFFFFFFFFFFFFFF0) + 1;
        this->write_config_32(0x10 + (4 * bar), low);
        this->write_config_32(0x10 + (4 * bar + 1), high);
    } else {
        Log::printk(Log::LogLevel::WARNING,
                    "pci: Unsupported BAR type (probably 16-bit or IO)\n");
    }
    return pcibar;
}

bool Device::is_claimed()
{
    return this->claimed;
}

void Device::claim()
{
    if (this->claimed) {
        Log::printk(
            Log::LogLevel::WARNING,
            "pci: Attempting to claim device that is already claimed!\n");
        return;
    }
    this->claimed = true;
}

void Device::unclaim()
{
    if (!this->claimed) {
        Log::printk(Log::LogLevel::WARNING,
                    "pci: Attempting to unclaim device that is not claimed!\n");
        return;
    }
    this->claimed = false;
}
} // namespace PCI
