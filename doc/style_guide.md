# Style Guide
This (WIP) document describes the various conventions in use in the Quark kernel. C++ is extremely lax in enforcing how things should look and be written, in contrast to languages such as Java that enforce many rules in naming, methodology, etc.

While this gives the programmer great freedom in how to code, it also can cause significant problems when bringing together multiple people into one codebase. Everyone has their own individual preferences, and without a consistent style guide the code in Quark would be a jumble of different styles that makes reading code difficult. This in turn makes finding bugs and adding features difficult as others often have to struggle with deciphering the existing code.

Following the conventions in this guide will help ensure that all contributors will be able to understand the code written by others and hopefully help make the codebase more maintainable.

# Acknowledgements
The layout of this style guide is based on the [Google Style Guide](https://google.github.io/styleguide/cppguide.html).

# C++ Version
The current C++ version used by the cross-compiler for Quark is C++17.

There are currently no plans to target C++20 as no compiler fully supports it yet, so C++20 specific features should NOT be used.

In an effort to maintain compiler portability, code should try to avoid using specific compiler features.

# File Names and Layout
## Headers
Headers should end with the `.h` suffix to indicate that they are header files. Furthermore, they should be located in the `include/` directory if they are architecture independent or in the `arch/include/<arch>` folder if they are for a specific architecture.

## C++ Files
Regular C++ files should end with the `.cc` suffix. They should go in the appropriate subsystem folder or under `arch/<arch>`.

## Layout
In general every `.cc` file should have a corresponding `.h` file, and their locations should be mirrored in the source code tree.

For example, if you are adding a file called `e1000.cc` to `drivers/net`, the header file `e1000.h` should go under `include/drivers/net`.

Similarly, if you are adding a file called `tsc.cc` to `arch/x86/drivers/clock`, the header file `tsc.h` should go under `arch/include/x86/drivers/net`.

# Header File Rules
## Self contained
Every header file should either forward declare or include everything that it needs.

## Forward Declaration
In general, try to use forward declaration where possible to avoid including unnecessary headers. If including headers when not needed, this can lead to a large number of files being recompiled if a single header file is touched.

Of course, this becomes more complicated when dealing with templates, so use your best judgement.

## Include Guards
Always use include guards to prevent headers from being included multiple times and causing multiple definition errors.

Whereas other projects recommend the standard include guards in the form of:

```c++
#ifndef PATH_TO_HEADER_H
#define PATH_TO_HEADER_H

// Other code

#endif
```

in Quark you should **always** use pragma guards:

```c++
#pragma once
```

While there is some debate over which form is better, `#pragma once` has the benefit of ensuring that the include guards will never collide and also reduces the chances of typos.

Since the drawbacks of pragma guards seem to be limited to the compiler failing to recognize that two identical files are actually the same under certain corner cases that never actually happen for all intents and purposes, use pragma guards.
