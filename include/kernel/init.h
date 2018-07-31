#pragma once

typedef int (*initcall_t)(void);

enum class InitLevel : int {
    EARLY,
    CORE,
    ARCH,
    SUBSYS,
    FS,
    DEVICE,
    LATE,
};

#define __define_initcall(fn, id)  \
    initcall_t __initcall_##id##fn \
        __attribute__((section(".initcall" #id), used)) = fn;

#define EARLY_INITCALL(fn) __define_initcall(fn, 1)
#define CORE_INITCALL(fn) __define_initcall(fn, 2)
#define ARCH_INITCALL(fn) __define_initcall(fn, 3)
#define SUBSYS_INITCALL(fn) __define_initcall(fn, 4)
#define FS_INITCALL(fn) __define_initcall(fn, 5)
#define DEVICE_INITCALL(fn) __define_initcall(fn, 6)
#define LATE_INITCALL(fn) __define_initcall(fn, 7)

void do_initcall(InitLevel level);
