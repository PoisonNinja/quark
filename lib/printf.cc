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

#include <lib/printf.h>
#include <lib/string.h>

struct parameters {
    char *buffer;
    uint8_t base;
    uint8_t uppercase;
    bool is_negative;
    int padding;
    int zero;
};

struct printf_data {
    char *buffer;
    size_t max;
    size_t used;
    void (*putcf)(struct printf_data *, char);
};

static void snprintf_putcf(struct printf_data *data, char c)
{
    if (data->used < data->max)
        data->buffer[data->used++] = c;
}

static void sprintf_putcf(struct printf_data *data, char c)
{
    data->buffer[data->used++] = c;
}

static void puts(struct parameters *params, struct printf_data *data,
                 const char *message)
{
    const char *tmp = message;
    int n = params->padding;
    while (*tmp++ && n)
        n--;
    if (!params->zero) {
        while (n--)
            data->putcf(data, ' ');
    }
    if (params->zero) {
        while (n--)
            data->putcf(data, '0');
    }
    while (*message) {
        data->putcf(data, *message++);
    }
}

static void ull2a(unsigned long long num, struct parameters *params)
{
    char *buffer = params->buffer;
    if (params->is_negative)
        *buffer++ = '-';
    int n = 0;
    unsigned long long d = 1;
    while (num / d >= params->base)
        d *= params->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= params->base;
        if (n || dgt > 0 || d == 0) {
            *buffer++ =
                dgt + (dgt < 10 ? '0' : (params->uppercase ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buffer = 0;
}

static void ll2a(int num, struct parameters *p)
{
    if (num < 0) {
        num = -num;
        p->is_negative = true;
    }
    ull2a(num, p);
}

static void ul2a(unsigned long num, struct parameters *params)
{
    char *buffer = params->buffer;
    if (params->is_negative)
        *buffer++ = '-';
    int n = 0;
    unsigned long d = 1;
    while (num / d >= params->base)
        d *= params->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= params->base;
        if (n || dgt > 0 || d == 0) {
            *buffer++ =
                dgt + (dgt < 10 ? '0' : (params->uppercase ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buffer = 0;
}

static void l2a(int num, struct parameters *p)
{
    if (num < 0) {
        num = -num;
        p->is_negative = true;
    }
    ul2a(num, p);
}

static void ui2a(unsigned int num, struct parameters *params)
{
    char *buffer = params->buffer;
    if (params->is_negative)
        *buffer++ = '-';
    int n = 0;
    unsigned int d = 1;
    while (num / d >= params->base)
        d *= params->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= params->base;
        if (n || dgt > 0 || d == 0) {
            *buffer++ =
                dgt + (dgt < 10 ? '0' : (params->uppercase ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buffer = 0;
}

static void i2a(int num, struct parameters *p)
{
    if (num < 0) {
        num = -num;
        p->is_negative = true;
    }
    ui2a(num, p);
}

static int a2u(char *src)
{
    unsigned int num = 0;
    int digit;
    while ((digit = *src++ - '0') >= 0) {
        if (digit > 10)
            break;
        num = num * 10 + digit;
    }
    return num;
}

static void printf_format(struct printf_data *data, const char *format,
                          va_list arg)
{
    struct parameters params;
    String::memset(&params, 0, sizeof(struct parameters));
    const char *pos = format;
    while (*pos != '\0') {
        if (*pos == '%') {
            // 22 chars for max value of long long in octal + 1 for null
            char buffer[23];
            params.buffer = buffer;
            String::memset(buffer, 0, 23);
            uint8_t lng = 0;
            pos++;
            if (*pos == '0') {
                params.zero = 1;
                pos++;
            }
            if (*pos >= '0' && *pos <= '9') {
                int i = 0;
                char padding[5];
                while (*pos >= '0' && *pos <= '9') {
                    padding[i++] = *pos++;
                }
                params.padding = a2u(padding);
            }
            if (*pos == 'l') {
                lng++;
                pos++;
                if (*pos == 'l') {
                    lng++;
                    pos++;
                }
            }
            switch (*pos++) {
                case 'c':
                    data->putcf(data, (char)va_arg(arg, int));
                    break;
                case 'd':
                case 'i':
                    params.base = 10;
                    if (lng == 0)
                        i2a(va_arg(arg, unsigned int), &params);
                    else if (lng == 1)
                        l2a(va_arg(arg, unsigned long), &params);
                    else if (lng == 2)
                        ll2a(va_arg(arg, unsigned long long), &params);
                    puts(&params, data, params.buffer);
                    break;
                case 'u':
                    params.base = 10;
                    if (lng == 0)
                        ui2a(va_arg(arg, unsigned int), &params);
                    else if (lng == 1)
                        ul2a(va_arg(arg, unsigned long), &params);
                    else if (lng == 2)
                        ull2a(va_arg(arg, unsigned long long), &params);
                    puts(&params, data, params.buffer);
                    break;
                case 'o':
                    params.base = 8;
                    ui2a(va_arg(arg, unsigned int), &params);
                    puts(&params, data, params.buffer);
                    break;
                case 's':
                    puts(&params, data, va_arg(arg, char *));
                    break;
                case 'p':
                    puts(&params, data, "0x");
                    params.zero = 1;
#if __SIZEOF_POINTER__ <= __SIZEOF_INT__
                    params.padding = 8;
                    lng = 0;
#elif __SIZEOF_POINTER__ <= __SIZEOF_LONG__
                    params.padding = 16;
                    lng = 1;
#elif __SIZEOF_POINTER__ <= __SIZEOF_LONG_LONG__
                    params.padding = 16;
                    lng = 2;
#endif
                case 'X':
                    params.uppercase = 1;
                case 'x':
                    params.base = 16;
                    if (lng == 0)
                        ui2a(va_arg(arg, unsigned int), &params);
                    else if (lng == 1)
                        ul2a(va_arg(arg, unsigned long), &params);
                    else if (lng == 2)
                        ull2a(va_arg(arg, unsigned long long), &params);
                    puts(&params, data, params.buffer);
                    break;
                case '%':
                    data->putcf(data, '%');
                    break;
            }
            String::memset(&params, 0, sizeof(struct parameters));
        } else {
            data->putcf(data, *pos++);
        }
    }
}

int vsprintf(char *s, const char *format, va_list arg)
{
    struct printf_data data = {
        .buffer = s,
        .putcf = sprintf_putcf,
    };
    printf_format(&data, format, arg);
    s[data.used] = '\0';
    return data.used;
}

int sprintf(char *s, const char *format, ...)
{
    va_list args;
    int ret;
    va_start(args, format);
    ret = vsprintf(s, format, args);
    va_end(args);
    return ret;
}

int vsnprintf(char *s, size_t size, const char *format, va_list arg)
{
    struct printf_data data = {
        .buffer = s,
        .max = size,
        .putcf = snprintf_putcf,
    };
    printf_format(&data, format, arg);
    s[data.used] = '\0';
    return data.used;
}

int snprintf(char *s, size_t size, const char *format, ...)
{
    va_list args;
    int ret;
    va_start(args, format);
    ret = vsnprintf(s, size, format, args);
    va_end(args);
    return ret;
}
