#include <arch/cpu/feature.h>
#include <kernel.h>
#include <lib/string.h>

namespace CPU
{
namespace X64
{
static void cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx)
{
    __asm__("cpuid"
            : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
            : "0"(*eax), "2"(*ecx)
            : "memory");
}

void detect_intel(Core& cpu)
{
    struct {
        uint32_t eax, ebx, ecx, edx;
    } regs;
    regs.eax = 0x80000007;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    if (regs.edx & (1 << 8)) {
        set_feature(cpu, X86_FEATURE_CONSTANT_TSC);
        set_feature(cpu, X86_FEATURE_NONSTOP_TSC);
    }
}

void detect(Core& cpu)
{
    struct {
        uint32_t eax, ebx, ecx, edx;
    } regs;

    regs.eax = 0x00000000;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    *(uint32_t*)&cpu.vendor[0] = regs.ebx;
    *(uint32_t*)&cpu.vendor[4] = regs.edx;
    *(uint32_t*)&cpu.vendor[8] = regs.ecx;
    // Manually add null terminator to prevent buffer overruns
    cpu.vendor[12] = '\0';

    regs.eax = 0x00000001;
    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
    cpu.stepping = regs.eax & 0xF;
    cpu.type = regs.eax & 0x3000;
    cpu.family = (regs.eax & 0xF00) + (regs.eax & 0xFF00000);
    cpu.model = ((regs.eax & 0xF0000) << 4) + (regs.eax & 0xF0);
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

    if (highest_function < 0x80000001) {
        Log::printk(
            Log::WARNING,
            "CPU does not support extended feature set past 0x80000001\n");
    } else {
        regs.eax = 0x80000001;
        cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
        cpu.features[cpuid_8000_0001_edx] = regs.edx;
        cpu.features[cpuid_8000_0001_ecx] = regs.ecx;

        if (highest_function < 0x80000004) {
            Log::printk(
                Log::WARNING,
                "CPU does not support extended feature set past 0x80000004\n");
        } else {
            regs.eax = 0x80000002;
            cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
            *(uint32_t*)&cpu.name[0] = regs.eax;
            *(uint32_t*)&cpu.name[4] = regs.ebx;
            *(uint32_t*)&cpu.name[8] = regs.ecx;
            *(uint32_t*)&cpu.name[12] = regs.edx;
            regs.eax = 0x80000003;
            cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
            *(uint32_t*)&cpu.name[16] = regs.eax;
            *(uint32_t*)&cpu.name[20] = regs.ebx;
            *(uint32_t*)&cpu.name[24] = regs.ecx;
            *(uint32_t*)&cpu.name[28] = regs.edx;
            regs.eax = 0x80000004;
            cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
            *(uint32_t*)&cpu.name[32] = regs.eax;
            *(uint32_t*)&cpu.name[36] = regs.ebx;
            *(uint32_t*)&cpu.name[40] = regs.ecx;
            *(uint32_t*)&cpu.name[44] = regs.edx;

            if (highest_function < 0x80000007) {
                Log::printk(Log::WARNING, "CPU does not support extended "
                                          "feature set past 0x80000007\n");
            } else {
                regs.eax = 0x80000007;
                cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
                cpu.features[cpuid_8000_0007_ebx] = regs.ebx;

                if (highest_function < 0x80000008) {
                    Log::printk(Log::WARNING, "CPU does not support extended "
                                              "feature set past 0x80000008\n");
                } else {
                    regs.eax = 0x80000008;
                    cpuid(&regs.eax, &regs.ebx, &regs.ecx, &regs.edx);
                    cpu.features[cpuid_8000_0008_ebx] = regs.ebx;

                    if (highest_function < 0x8000000A) {
                        Log::printk(Log::WARNING,
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

    if (!String::strncmp("GenuineIntel", cpu.vendor, 13)) {
        detect_intel(cpu);
    } else {
        Log::printk(
            Log::WARNING,
            "Unknown CPU vendor, not performing extended feature detection.\n");
    }
}

void print(Core& cpu)
{
    Log::printk(Log::INFO, "CPU Vendor: %s\n", cpu.vendor);
    Log::printk(Log::INFO, "CPU Name: %s\n", cpu.name);
    Log::printk(Log::INFO, "CPU Stepping: %X\n", cpu.stepping);
    Log::printk(Log::INFO, "CPU Type: %X\n", cpu.type);
    Log::printk(Log::INFO, "CPU Family: %X\n", cpu.family);
    Log::printk(Log::INFO, "CPU Model: %X\n", cpu.model);
}
}
}
