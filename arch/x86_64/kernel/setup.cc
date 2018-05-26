/*
 * Copyright (C) 2017 Jason Lu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch/cpu/cpu.h>
#include <arch/kernel/multiboot2.h>
#include <boot/info.h>
#include <cpu/interrupt.h>
#include <drivers/tty/serial.h>
#include <kernel.h>
#include <lib/libcxx.h>

extern void kmain(struct Boot::info &info);

namespace X64
{
extern "C" {
void *__constructors_start;
void *__constructors_end;

void *__kernel_start;
void *__kernel_end;
}

static Serial serial_console;
static struct Boot::info info;

void init(uint32_t magic, struct multiboot_fixed *multiboot)
{
    Log::register_log_output(serial_console);
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        Log::printk(Log::ERROR, "Multiboot magic number does not match!\n");
    }
    info.architecture_data = multiboot;
    info.kernel_start = reinterpret_cast<addr_t>(&__kernel_start);
    info.kernel_end = reinterpret_cast<addr_t>(&__kernel_end);
    struct multiboot_tag *tag;
    for (tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<addr_t>(multiboot) + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = reinterpret_cast<struct multiboot_tag *>(
             reinterpret_cast<multiboot_uint8_t *>(tag) +
             ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                info.cmdline =
                    reinterpret_cast<struct multiboot_tag_string *>(tag)
                        ->string;
                break;
            // case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            //     Log::printk(
            //         Log::INFO, "    Boot loader name = %s\n",
            //         (reinterpret_cast<struct multiboot_tag_string
            //         *>(tag))
            //             ->string);
            //     break;
            case MULTIBOOT_TAG_TYPE_MODULE:
                info.initrd_start =
                    (reinterpret_cast<struct multiboot_tag_module *>(tag))
                        ->mod_start;
                info.initrd_end =
                    (reinterpret_cast<struct multiboot_tag_module *>(tag))
                        ->mod_end;
                break;
                // case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                //     Log::printk(
                //         Log::INFO, "    Lower memory = %uKB, Upper memory
                //         = %uKB\n", (reinterpret_cast<struct
                //         multiboot_tag_basic_meminfo *>(
                //              tag))
                //             ->mem_lower,
                //         (reinterpret_cast<struct
                //         multiboot_tag_basic_meminfo
                //         *>(
                //              tag))
                //             ->mem_upper);
                //     break;
                // case MULTIBOOT_TAG_TYPE_BOOTDEV:
                //     Log::printk(
                //         Log::INFO, "    Boot device 0x%x,%u,%u\n",
                //         (reinterpret_cast<struct multiboot_tag_bootdev
                //         *>(tag))
                //             ->biosdev,
                //         (reinterpret_cast<struct multiboot_tag_bootdev
                //         *>(tag))
                //             ->slice,
                //         (reinterpret_cast<struct multiboot_tag_bootdev
                //         *>(tag))
                //             ->part);
                //     break;
                // case MULTIBOOT_TAG_TYPE_MMAP: {
                //     multiboot_memory_map_t *mmap;
                //     Log::printk(Log::INFO, "    Memory map:\n");
                //     for (mmap = (reinterpret_cast<struct
                //     multiboot_tag_mmap
                //     *>(tag))
                //                     ->entries;
                //          reinterpret_cast<multiboot_uint8_t *>(mmap) <
                //          reinterpret_cast<multiboot_uint8_t *>(tag) +
                //          tag->size; mmap =
                //          reinterpret_cast<multiboot_memory_map_t *>(
                //              reinterpret_cast<addr_t>(mmap) +
                //              (reinterpret_cast<struct multiboot_tag_mmap
                //              *>(tag))
                //                  ->entry_size))
                //         Log::printk(Log::INFO,
                //                     "        Base = 0x%08x%08x,"
                //                     " Length = 0x%08x%08x, Type =
                //                     0x%x\n",
                //                     static_cast<addr_t>(mmap->addr >>
                //                     32), static_cast<addr_t>(mmap->addr &
                //                     0xffffffff),
                //                     static_cast<addr_t>(mmap->len >> 32),
                //                     static_cast<addr_t>(mmap->len &
                //                     0xffffffff),
                //                     static_cast<addr_t>(mmap->type));
                // } break;
                // default:
                //     Log::printk(Log::INFO, "    Unknown/unhandled\n");
        }
    }
    CPU::X64::init();
    kmain(info);
}

extern "C" {
void asm_to_cxx_trampoline(uint32_t magic, struct multiboot_fixed *multiboot)
{
    libcxx::constructors_initialize(&__constructors_start, &__constructors_end);
    X64::init(magic, multiboot);
}
}
}  // namespace X64
