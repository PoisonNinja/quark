#pragma once

[[noreturn]] void __assert(const char* message, const char* file, int line);

#ifdef QUARK_DEBUG
#define assert(EX) (void)((EX) || (__assert(#EX, __FILE__, __LINE__), false))
#else
#define assert(EX)
#endif
