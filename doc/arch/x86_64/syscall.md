# x86_64 System Call ABI
This document describes the system call ABI for x86_64. Since Quark is in development, the ABI is subject to change without notice, but this document will be updated accordingly. Furthermore, ABI changes are a last resort and are rarely changed.

## Description
x86_64 largely follows the x86_64 Linux system call ABI which in turn follows the x86_64 System V system call ABI.

On x86_64, Quark uses the [syscall](https://www.felixcloutier.com/x86/SYSCALL.html) instruction which is much more performant vs. the traditional `int $0x80` method.

Here is a brief overview of the register usages:
* RAX: System call #
* RDI: Argument #1
* RSI: Argument #2
* RDX: Argument #3
* R10: Argument #4
* R8: Argument #5

Notice that RDX is used instead of RCX. This is because `syscall` clobbers RCX to store the return address, so RDX is used instead.

## Usage
The reference newlib implementation for Pepper contains a C library wrapper for making a system call. The usage of that wrapper is preferred because it hides any ABI changes and prevents it from breaking your programs.

However, to manually make a system call:
* Set RAX to your desired system call #
* Pass in the parameters to the system call as you would for a regular x86_64 function (RDI, RSI, RDX, RCX, R8), substituting RDX for RCX
* Use `syscall`
* Return