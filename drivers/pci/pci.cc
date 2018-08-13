#include <arch/drivers/io.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/pci_list.h>
#include <kernel.h>

namespace
{
constexpr uint16_t config_address = 0xCF8;
constexpr uint16_t config_data    = 0xCFC;

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
} // namespace

namespace PCI
{
void init()
{
    Log::printk(Log::LogLevel::INFO, "pci: Initializing...\n");
    probe();
}
} // namespace PCI