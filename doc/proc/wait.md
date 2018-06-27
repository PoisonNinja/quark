# Waiting
Quark implements rudimentary support for waiting for events in the kernel. This document describes how waiting works and how to use it in the kernel (WIP).

## Design
The core of the waiting subsystem is the Waiter object. This object keeps track of active waiters and their conditions that cause the wait to be terminated.

In Quark, the conditions are actually tokens which are used to uniquely identify event dispatchers.

Here is a typical flow:
* Thread wants to wait on a file
* It creates a wait object with a random token (probably the inode + dev number) and hands it to the vnode to use.
* The thread then puts itself to sleep
* Upon a write to the file, the vnode layer checks if a token is set and broadcasts that token
* The wait subsystem will call the check() function for each waiter with the token passed in. Once our thread gets called to check, it will return true since we have a match
* The thread then resumes execution

Waiters can be inherited from. This is useful if a thread wants to do more than simply comparing tokens. For example, a file waiter can store the Vnode pointer and then later cast the token back to a vnode and check ino and dev.