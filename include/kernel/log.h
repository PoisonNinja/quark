#pragma once

#include <lib/list.h>
#include <types.h>

class log_output
{
public:
    virtual size_t write(const char*, size_t) = 0;
    libcxx::node<log_output> node;
};

namespace log
{
enum class log_level : int {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CONTINUE,
};
size_t printk(log_level level, const char* format, ...);

void register_log_output(log_output& device);
} // namespace log
