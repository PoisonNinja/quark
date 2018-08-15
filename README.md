# Quark
Quark is an experimental kernel written in C++ that's mainly for me to play around with. It serves as the kernel for the [Pepper](https://github.com/PoisonNinja/Pepper). It currently has support for both x86_64 and i686.

# Getting Started
## Prerequisites
Make sure that you have followed the toolchain setup instructions in the Pepper README.md, and that you have checked out the latest Quark code as a submodule.

## Build system
Quark uses the CMake build system. As such, you will need to generate your own build files. To avoid cluttering the source code directory, Quark requires out-of-source builds. Create a subdirectory called `build` in the Quark folder. Change directory into the folder.

To generate the build files, run a variation of the command below:
```
cmake .. -G <your generator>
```

Replace `<your generator>` with your preferred build system. Due to the way that we enforce dependencies for linker scripts and out-of-tree objects, only Make and Ninja are supported as build generators (see [here](https://cmake.org/gitweb?p=cmake.git;a=blob;f=Tests/BuildDepends/CMakeLists.txt;h=39a5131fed09be30e935a68c9c4008390fa6fe1e;hb=c4b0e96c37b1d030bf63bc9cf005a50329e7e71c#l37)).

If you wish to build with debug support and logging, append `-DCMAKE_BUILD_TYPE=Debug` to the command to generate the build files.

## Using the build system
Quark is primarily designed to interface with the Pepper build system. Therefore, you should use that most of the time. However, you can always use Quark's own build files.

# License
Quark itself is licensed under the 3-clause BSD license, and the full text can be found in the file LICENSE.

Quark's allocator is durand's liballoc, which has been released into the public domain.

Some portions of the operating system are based on Sortie's [Sortix](https://gitlab.com/Sortix/Sortix) operating system. While no code was copied, the design is pretty similar which I feel warrants attribution. Therefore, please note that portions of the filesystem and process management code is partially licensed under the ISC license.

