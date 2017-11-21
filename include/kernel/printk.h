#pragma once

#include <types.h>

class Log
{
public:
    enum {
        DEBUG = 0,
        INFO,
        WARNING,
        ERROR,
        CONTINUE,
    };

    static size_t printk(int level, const char *format, ...);
};
