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

#include <config.h>
#include <kernel.h>
#include <kernel/log.h>
#include <kernel/time/time.h>
#include <lib/printf.h>
#include <lib/string.h>

namespace log
{
constexpr size_t printk_max = CONFIG_PRINTK_MAX;

static libcxx::list<log_output, &log_output::node> output;

static const char* colors[] = {
    "\e[36m", // Blue for debug
    "\e[32m", // Green for info
    "\e[33m", // Yellow for warning
    "\e[31m", // Red for error
};

static char printk_buffer[log::printk_max];

size_t printk(log_level level, const char* format, ...)
{
#ifndef QUARK_DEBUG
    if (level != log::log_level::DEBUG) {
#endif
        size_t r = 0;
        if (level < log::log_level::CONTINUE) {
            libcxx::memset(printk_buffer, 0, log::printk_max);
            struct time::timespec spec = time::now();
            r = libcxx::snprintf(printk_buffer, log::printk_max,
                                 "%s[%05llu.%09llu]%s ",
                                 colors[static_cast<int>(level)], spec.tv_sec,
                                 spec.tv_nsec, "\e[39m");
            for (auto& i : output) {
                i.write(printk_buffer, r);
            }
        }
        libcxx::memset(printk_buffer, 0, log::printk_max);
        va_list args;
        va_start(args, format);
        r = libcxx::vsnprintf(printk_buffer, log::printk_max, format, args);
        va_end(args);
        for (auto& i : output) {
            i.write(printk_buffer, r);
        }
        return r;
#ifndef QUARK_DEBUG
    } else {
        return 0;
    }
#endif
}

void register_log_output(log_output& device)
{
    output.push_back(device);
}
} // namespace log
