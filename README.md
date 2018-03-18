# Quark
Quark is an experimental kernel written in C++ that's mainly for me to play around with. It serves as the kernel for the [Pepper](https://github.com/PoisonNinja/Pepper).

# Getting Started
## Prerequisites
Make sure that you have followed the toolchain setup instructions in the Pepper README.md, and that you have checked out the latest Quark code as a submodule.

## Build system
Quark uses the CMake build system. As such, you will need to generate your own build files. To avoid cluttering the source code directory, Quark requires out-of-source builds. Create a subdirectory called `build` in the Quark folder. Change directory into the folder.

To generate the build files, run a variation of the command below:
```
cmake .. -G <your generator>
```

Replace `<your generator>` with your preferred build system, such as Make or Ninja. For a list, see the CMake documentation.

If you wish to build with debug support and logging, append `-DCMAKE_BUILD_TYPE=Debug` to the command to generate the build files.

## Using the build system
Quark is primarily designed to interface with the Pepper build system. Therefore, you should use that most of the time. However, you can always use Quark's own build files.

# Other notes
Quark was intended to be architecture portable, but recent efforts have shown that this is harder than previously thought. While I have not abandoned this goal, I have, in some cases, integrated architecture-specific code with generic code for the sake of clarity. This is especially apparent in the thread switching code and the asm("hlt")'s littered around the kernel.

As the design of the kernel slowly takes shape, I will start moving things into the proper places. The eventual goal is to undertake a port to another architecture (perhaps Aarch64) to ensure that the kernel is truly seperated between architecture dependant and independant code.
