# Configuration
Thanks to CMake, Quark has the ability to let the developer configure certain features in Quark during build time, similar to how to KConfig system works for Linux.

Below are instructions for adding a new configuration option so that CMake is aware of it and how to use the option in your code.

# Adding an option
In a CMakeLists.txt file, add the following line:

```
set(CONFIG_<NAME> "<DEFAULT VALUE>" CACHE <TYPE> "<DESCRIPTION OF OPTION>" )
```

While you can technically place that anywhere, I recommend that you place it at the top of the CMakeLists.txt file that also builds your source code so it's easier to find.

The option can also be named anything, but as a convention custom options are prefixed with CONFIG_

The TYPE can be any of the types supported by CMake as listed [here](https://cmake.org/cmake/help/v3.0/command/set.html). As a general guideline, the STRING type should satisfy most needs.

Please make the description as descriptive as possible so that users can know what exactly they're configuring.

You will also need to add the following string to `config.h.in` in the root directory to allow source code to access it

```
#cmakedefine CONFIG_<NAME> @CONFIG_<NAME>@
```

# Configuring the option
There are several ways to configure your new option. The first is to define it during build system setup:
```
cmake .. -DYOUR_OPTION_NAME=OPTION_VALUE
```

The other alternative is to use `ccmake` which allows you to graphically configure the option similar to how the ncurses interface for KConfig works.

To use it, point `ccmake` to the build directory, e.g.:
```
ccmake build/
```

# Using the option
You can use the option in both the build system and in source code.

## Build system
To use it in the build system, you can simply use it as any other CMake variable. For example:

```
if(CONFIG_FOO)
    target_sources(quark.kernel
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/foo.cc
    )
endif()
```

## Source code
Quark converts CMake configuration options defined in `config.h.in` into `config.h`. To use that, simply `#include <config.h>`.

Like Linux, configuration options are defined as C macros, so you can do things like:
```C
#ifdef CONFIG_FOO
    // Do something
#endif
```

or

```C
#if CONFIG_BAR == 1024
    // Do something else
#endif
```

or

```C
log::printk(log::log_level::INFO, "CONFIG_BAZ is %d\n", CONFIG_BAZ);
```
