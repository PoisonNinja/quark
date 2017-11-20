#pragma once

#include <lib/list.h>
#include <types.h>

struct console {
    size_t (*write)(const char* buffer, size_t size);
    struct list_element list;
    char* name;
};

extern status_t console_register(struct console* console);

extern status_t console_write(const char* buffer, size_t size);
