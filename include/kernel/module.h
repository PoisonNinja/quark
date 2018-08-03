#pragma once

#include <lib/list.h>
#include <types.h>

// Each key can only be defined once
#define __define_modinfo(key, value)                                          \
    const char __modinfo_##key[] __attribute__((section(".modinfo"), used)) = \
        #key "=" value

#define MODULE_AUTHOR(who) __define_modinfo(author, who)
#define MODULE_DESCRIPTION(what) __define_modinfo(description, what)
#define MODULE_VERSION(when) __define_modinfo(version, when)
#define MODULE_NAME(what) __define_modinfo(name, what)

bool load_module(void* binary);
bool unload_module(const char* name);