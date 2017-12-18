#pragma once

#include <lib/list.h>
#include <types.h>

namespace Log
{
class LogOutput
{
public:
    virtual size_t Write(const char*, size_t)
    {
        return 0;
    };
    Node<LogOutput> node;
};

enum {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CONTINUE,
};
size_t Printk(int level, const char* format, ...);

void RegisterLogOutput(LogOutput& device);
}  // namespace Log
