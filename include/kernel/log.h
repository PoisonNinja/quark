#pragma once

#include <lib/list.h>
#include <types.h>

class LogOutput
{
public:
    virtual size_t write(const char*, size_t) = 0;
    libcxx::node<LogOutput> node;
};

namespace Log
{
enum class LogLevel : int {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CONTINUE,
};
size_t printk(LogLevel level, const char* format, ...);

void register_log_output(LogOutput& device);
} // namespace Log
