# Strings
Quark has a fairly useful implementation of a string class. This document describes usage and warnings about the usage.

# Usage
In general you can use libcxx::string similar to std::string by including lib/string.h. Note that it is under the libcxx namespace, not std.

However, usage of std::string should be restricted to kernel subsystems such as the filesystem and NOT for drivers.

# Warnings
There are a couple of things to watch out for when using std::string:
* Do NOT declare std::string as a global variable (e.g. outside of a function) because constructors are called before the memory allocator goes online.

    If the std::string is initialized with a short string this may not be an issue because SSO will ensure that it uses static memory, but longer strings will cause it to try to use new/delete which will cause a panic.
* Make sure that you include the right header (lib/string.h, not string). Including the wrong header will cause weird issues, especially when the kernel is being linked. Usually including the wrong header will immediately give an error because Quark uses libcxx instead of std
* For high performance/timing sensitive code (e.g. drivers), stick to using char*.
