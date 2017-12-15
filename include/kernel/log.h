#pragma once

#include <lib/list.h>
#include <types.h>

class LogOutput
{
public:
    virtual size_t write(const char*, size_t)
    {
        return 0;
    };
    Node<LogOutput> node;
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

void registerLogOutput(LogOutput& device);
}  // namespace Log
