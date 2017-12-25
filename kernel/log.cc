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

#include <kernel.h>
#include <kernel/log.h>
#include <lib/printf.h>
#include <lib/string.h>

namespace Log
{
#define PRINTK_MAX 1024

static List<LogOutput, &LogOutput::node> output;

static const char* colors[] = {
    "\e[36m",  // Blue for debug
    "\e[32m",  // Green for info
    "\e[33m",  // Yellow for warning
    "\e[31m",  // Red for error
};

static char printk_buffer[PRINTK_MAX];

size_t printk(int level, const char* format, ...)
{
#ifndef QUARK_DEBUG
    if (level != Log::DEBUG) {
#endif
        size_t r = 0;
        if (level < Log::CONTINUE) {
            String::memset(printk_buffer, 0, PRINTK_MAX);
            // time_t t = ktime_get();
            time_t sec = 0;   // t / NSEC_PER_SEC;
            time_t nsec = 0;  // t % NSEC_PER_SEC;
            r = snprintf(printk_buffer, PRINTK_MAX, "%s[%05lu.%09lu]%s ",
                         colors[level], sec, nsec, "\e[39m");
            for (auto& i : output) {
                i.write(printk_buffer, r);
            }
        }
        String::memset(printk_buffer, 0, PRINTK_MAX);
        va_list args;
        va_start(args, format);
        r = vsnprintf(printk_buffer, PRINTK_MAX, format, args);
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

void register_log_output(LogOutput& device)
{
    output.push_back(device);
}
}  // namespace Log
