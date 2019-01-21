#pragma once

#include <lib/list.h>
#include <proc/elf.h>
#include <types.h>

struct module {
    module()
    {
        shdrs    = nullptr;
        sections = nullptr;
        init     = nullptr;
        fini     = nullptr;
        name = description = version = author = nullptr;
    }

    const char *name, *description, *version, *author;

    size_t shnum;

    // Dynamically allocated
    elf::elf_shdr* shdrs;
    addr_t* sections;

    // module entry points
    int (*init)();
    int (*fini)();

    libcxx::node<module> node;
};

// Each key can only be defined once
#define __define_modinfo(key, value)                                           \
    const char __modinfo_##key[] __attribute__((section(".modinfo"), used)) =  \
        #key "=" value

#define MODULE_AUTHOR(who) __define_modinfo(author, who)
#define MODULE_DESCRIPTION(what) __define_modinfo(description, what)
#define MODULE_VERSION(when) __define_modinfo(version, when)
#define MODULE_NAME(what) __define_modinfo(name, what)

bool load_module(void* binary);
bool unload_module(const char* name);
