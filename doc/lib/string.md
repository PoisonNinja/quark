# Strings
Quark has a fairly useful implementation of a string class. This document describes usage and warnings about the usage.

# Usage
In general you can use libcxx::string similar to std::string by including lib/string.h. Note that it is under the libcxx namespace (not std) like every other library provided by Quark.

However, usage of libcxx::string should be restricted to kernel subsystems that are userspace facing such as the filesystem and NOT early-boot time code such as timekeeping and drivers.

# Warnings
There are a couple of things to watch out for when using libcxx::string:
* Do NOT declare libcxx::string as a global variable (e.g. outside of a function) because constructors are called before the memory allocator goes online.

    If the libcxx::string is initialized with a short string this may not be an issue because SSO will ensure that the string uses static memory, but longer strings will cause the string to try to use new/delete before the memory allocator is initialized which will cause a panic.

* Make sure that you include the right header (lib/string.h, not string). Including the wrong header will cause weird issues, especially when the kernel is being linked. Usually including the wrong header will immediately give an error because Quark uses libcxx instead of std

* For high performance/timing sensitive code (e.g. drivers), stick to using char*.
