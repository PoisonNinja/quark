#pragma once

#include <lib/list.h>
#include <types.h>

class LogOutput
{
public:
    virtual size_t write(const char* buffer, size_t size) = 0;
};

namespace Log
{
enum {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CONTINUE,
};
size_t printk(int level, const char* format, ...);

void RegisterLogOutput(LogOutput* output);
}  // namespace Log
