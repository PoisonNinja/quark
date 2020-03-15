# Quark
Quark is an experimental kernel written mainly in C++ (with some assembly) that is primarily intended for me to learn more about how kernels and hardware work.

While Quark is mainly targeted as the kernel for the [Pepper](https://github.com/PoisonNinja/Pepper) operating system, in theory any UNIX-compatible program can run on top of Quark (meaning something like GNU/Pepper feasible).

# Getting Started
## Prerequisites
Make sure that you have followed the toolchain setup instructions in the Pepper README.md, and that you have checked out the latest Quark code as a submodule.

## Build system
Quark uses the CMake build system. As such, you will need to generate your own build files. To avoid cluttering the source code directory, Quark requires out-of-source builds. Create a subdirectory called `build` in the Quark folder. Change directory into the folder.

To generate the build files, run a variation of the command below:
```
cmake .. -G <your generator>
```

Replace `<your generator>` with your preferred build system. Due to the way that we enforce dependencies for linker scripts and out-of-tree objects, only Make and Ninja are supported as build generators (see [here](https://cmake.org/gitweb?p=cmake.git;a=blob;f=Tests/BuildDepends/CMakeLists.txt;h=39a5131fed09be30e935a68c9c4008390fa6fe1e;hb=c4b0e96c37b1d030bf63bc9cf005a50329e7e71c#l37) for more details).

If you wish to build with debug support and logging, append `-DCMAKE_BUILD_TYPE=Debug` to the command to generate the build files.

## Using the build system
Quark is primarily designed to interface with the Pepper build system. Therefore, you should use that most of the time. However, you can always use Quark's own build files.

# License
Quark itself is licensed under the 3-clause BSD license, and the full text can be found in the file LICENSE.

Quark uses some code from third parties for its library implementations. The specific third-party code descriptions and license terms are stated both in the actual code and in LICENSE-3RD-PARTY.
