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

#include <arch/kernel/multiboot2.h>
#include <kernel.h>

extern void x86_64_initialize_serial(void);

void x86_64_init(uint32_t magic, struct multiboot_fixed *multiboot)
{
    x86_64_initialize_serial();
    printk(INFO, "x86_64 preinitialization...\n");
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printk(ERROR, "Multiboot magic number does not match!\n");
    }
    printk(INFO, "Multiboot information at %p with total size 0x%llX\n",
           multiboot, multiboot->total_size);
    struct multiboot_tag *tag;
    printk(INFO, "Parsing Multiboot tag information: \n");
    for (tag = (struct multiboot_tag *)((addr_t)multiboot + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag +
                                        ((tag->size + 7) & ~7))) {
        printk(INFO, "Tag 0x%d, Size 0x%x\n", tag->type, tag->size);
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                printk(INFO, "    Command line = %s\n",
                       ((struct multiboot_tag_string *)tag)->string);
                break;
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                printk(INFO, "    Boot loader name = %s\n",
                       ((struct multiboot_tag_string *)tag)->string);
                break;
            case MULTIBOOT_TAG_TYPE_MODULE:
                printk(INFO, "    Module at 0x%x - 0x%x. Command line %s\n",
                       ((struct multiboot_tag_module *)tag)->mod_start,
                       ((struct multiboot_tag_module *)tag)->mod_end,
                       ((struct multiboot_tag_module *)tag)->cmdline);
                break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                printk(INFO, "    Lower memory = %uKB, Upper memory = %uKB\n",
                       ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower,
                       ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper);
                break;
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                printk(INFO, "    Boot device 0x%x,%u,%u\n",
                       ((struct multiboot_tag_bootdev *)tag)->biosdev,
                       ((struct multiboot_tag_bootdev *)tag)->slice,
                       ((struct multiboot_tag_bootdev *)tag)->part);
                break;
            case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_memory_map_t *mmap;
                printk(INFO, "    Memory map:\n");
                for (mmap = ((struct multiboot_tag_mmap *)tag)->entries;
                     (multiboot_uint8_t *)mmap <
                     (multiboot_uint8_t *)tag + tag->size;
                     mmap = (multiboot_memory_map_t
                                 *)((addr_t)mmap +
                                    ((struct multiboot_tag_mmap *)tag)
                                        ->entry_size))
                    printk(INFO,
                           "        Base = 0x%08x%08x,"
                           " Length = 0x%08x%08x, Type = 0x%x\n",
                           (addr_t)(mmap->addr >> 32),
                           (addr_t)(mmap->addr & 0xffffffff),
                           (addr_t)(mmap->len >> 32),
                           (addr_t)(mmap->len & 0xffffffff),
                           (addr_t)mmap->type);
            } break;
            default:
                printk(INFO, "    Unknown/unhandled\n");
        }
    }
    for (;;)
        asm("hlt");
}
