#include <arch/cpu/feature.h>
#include <kernel.h>
#include <lib/string.h>

namespace
{
struct cpuid_regs {
    cpuid_regs()
        : eax(0)
        , ebx(0)
        , ecx(0)
        , edx(0){};
    uint32_t eax, ebx, ecx, edx;
};
} // namespace

namespace cpu
{
namespace x86_64
{
static void cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx)
{
    __asm__("cpuid"
            : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
            : "0"(*eax), "2"(*ecx)
            : "memory");
}

void detect_intel(core& cpu)
{
    /*
     * Linux based - Family 0x6, Model >= 0xE and Family 0xF, Model >= 0x3
     * support constant TSC
     */
    if ((cpu.family == 0x6 && cpu.model >= 0xE) ||
        (cpu.family == 0xF && cpu.model >= 0x3)) {
        set_feature(cpu, X86_FEATURE_CONSTANT_TSC);
    }
    // Intel specific CPUID extensions
    struct cpuid_regs regs;
    regs.eax = 0x80000007;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    if (regs.edx & (1 << 8)) {
        set_feature(cpu, X86_FEATURE_CONSTANT_TSC);
        set_feature(cpu, X86_FEATURE_NONSTOP_TSC);
    }
}

void detect(core& cpu)
{
    struct cpuid_regs regs;

    // CPU vendor
    regs.eax = 0x00000000;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    *(uint32_t*)&cpu.vendor[0] = regs.ebx;
    *(uint32_t*)&cpu.vendor[4] = regs.edx;
    *(uint32_t*)&cpu.vendor[8] = regs.ecx;
    // Manually add null terminator to prevent buffer overruns
    cpu.vendor[12] = '\0';

    regs.eax = 0x00000001;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    cpu.type = (regs.eax & 0x3000) >> 12;
    /*
     * If model is NOT equal to 15, family is extended family + family.
     * Otherwise just equal to family
     */
    if ((regs.eax & 0xF00) != 0xF00) {
        cpu.family = ((regs.eax & 0xF00) + (regs.eax & 0xFF00000)) >> 8;
    } else {
        cpu.family = (regs.eax & 0xF00) >> 8;
    }
    cpu.stepping = regs.eax & 0xF;
    /*
     * If model is equal to 6 or 15, model is extended model << 4 + model.
     * Otherwise just equal to model
     */
    if ((regs.eax & 0xF00) == 0xF00 || (regs.eax & 0xF00) == 0x600) {
        cpu.model = (((regs.eax & 0xF0000) >> 12) + ((regs.eax & 0xF0) >> 4));
    } else {
        cpu.model = (regs.eax & 0xF0) >> 4;
    }
    cpu.features[cpuid_1_edx] = regs.edx;
    cpu.features[cpuid_1_ecx] = regs.ecx;

    regs.eax = 0x00000006;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    cpu.features[cpuid_6_eax] = regs.eax;

    regs.eax = 0x00000007;
    regs.ecx = 0x00000000;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    cpu.features[cpuid_7_ebx] = regs.ebx;
    cpu.features[cpuid_7_ecx] = regs.ecx;
    cpu.features[cpuid_7_edx] = regs.edx;

    regs.eax = 0x0000000D;
    regs.ecx = 0x00000001;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    cpu.features[cpuid_D_1_eax] = regs.eax;

    regs.eax = 0x0000000F;
    regs.ecx = 0x00000000;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    cpu.features[cpuid_F_0_edx] = regs.edx;

    regs.eax = 0x0000000F;
    regs.ecx = 0x00000001;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    cpu.features[cpuid_F_1_edx] = regs.edx;

    regs.eax = 0x80000000;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    uint32_t highest_function = regs.eax;

    /*
     * CPUID EAX values past 0x80000000 are optional so we need to verify
     * they exist
     */
    if (highest_function < 0x80000001) {
        log::printk(
            log::log_level::WARNING,
            "CPU does not support extended feature set past 0x80000001\n");
    } else {
        regs.eax = 0x80000001;
        cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
        cpu.features[cpuid_8000_0001_edx] = regs.edx;
        cpu.features[cpuid_8000_0001_ecx] = regs.ecx;

        if (highest_function < 0x80000004) {
            log::printk(log::log_level::WARNING,
                        "CPU does not support extended feature set past "
                        "0x80000004\n");
        } else {
            regs.eax = 0x80000002;
            cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
            *(uint32_t*)&cpu.name[0]  = regs.eax;
            *(uint32_t*)&cpu.name[4]  = regs.ebx;
            *(uint32_t*)&cpu.name[8]  = regs.ecx;
            *(uint32_t*)&cpu.name[12] = regs.edx;
            regs.eax                  = 0x80000003;
            cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
            *(uint32_t*)&cpu.name[16] = regs.eax;
            *(uint32_t*)&cpu.name[20] = regs.ebx;
            *(uint32_t*)&cpu.name[24] = regs.ecx;
            *(uint32_t*)&cpu.name[28] = regs.edx;
            regs.eax                  = 0x80000004;
            cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
            *(uint32_t*)&cpu.name[32] = regs.eax;
            *(uint32_t*)&cpu.name[36] = regs.ebx;
            *(uint32_t*)&cpu.name[40] = regs.ecx;
            *(uint32_t*)&cpu.name[44] = regs.edx;

            if (highest_function < 0x80000007) {
                log::printk(log::log_level::WARNING,
                            "CPU does not support extended "
                            "feature set past 0x80000007\n");
            } else {
                regs.eax = 0x80000007;
                cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
                cpu.features[cpuid_8000_0007_ebx] = regs.ebx;

                if (highest_function < 0x80000008) {
                    log::printk(log::log_level::WARNING,
                                "CPU does not support extended "
                                "feature set past 0x80000008\n");
                } else {
                    regs.eax = 0x80000008;
                    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
                    cpu.features[cpuid_8000_0008_ebx] = regs.ebx;

                    if (highest_function < 0x8000000A) {
                        log::printk(log::log_level::WARNING,
                                    "CPU does not support extended "
                                    "feature set past 0x8000000A\n");
                    } else {
                        regs.eax = 0x8000000A;
                        cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
                        cpu.features[cpuid_8000_000A_edx] = regs.edx;
                    }
                }
            }
        }
    }

    // Vendor specific CPUID extensions
    if (!libcxx::strncmp("GenuineIntel", cpu.vendor, 13)) {
        log::printk(log::log_level::INFO, "Found supported CPU vendor\n");
        detect_intel(cpu);
    } else {
        log::printk(log::log_level::WARNING,
                    "Unknown CPU vendor, not performing extended feature "
                    "detection.\n");
    }
}

void print(core& cpu)
{
    log::printk(log::log_level::INFO, "CPU Vendor:   %s\n", cpu.vendor);
    log::printk(log::log_level::INFO, "CPU Name:     %s\n", cpu.name);
    log::printk(log::log_level::INFO, "CPU Stepping: %X\n", cpu.stepping);
    log::printk(log::log_level::INFO, "CPU Type:     %X\n", cpu.type);
    log::printk(log::log_level::INFO, "CPU Family:   %X\n", cpu.family);
    log::printk(log::log_level::INFO, "CPU Model:    %X\n", cpu.model);
}
} // namespace x86_64
} // namespace cpu
