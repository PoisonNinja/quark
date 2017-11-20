#pragma once

#include <types.h>

enum {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CONTINUE,
};

#define printk(level, ...) _printk(level, __VA_ARGS__)
size_t _printk(int level, const char *format, ...);
