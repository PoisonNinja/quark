#pragma once

#include <types.h>

namespace CPU
{
namespace X86Family
{
constexpr uint64_t msr_sysenter_cs = 0x174;
constexpr uint64_t msr_sysenter_esp = 0x175;
constexpr uint64_t msr_sysenter_eip = 0x176;

constexpr uint64_t msr_efer = 0xC0000080;
constexpr uint64_t msr_star = 0xC0000081;
constexpr uint64_t msr_lstar = 0xC0000082;
constexpr uint64_t msr_cstar = 0xC0000083;
constexpr uint64_t msr_fmask = 0xC0000084;

constexpr uint64_t msr_fs_base = 0xC0000100;
constexpr uint64_t msr_gs_base = 0xC0000101;
constexpr uint64_t msr_kernel_gs_base = 0xC0000102;
}  // namespace X86Family
}  // namespace CPU