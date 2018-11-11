#pragma once

#include <types.h>

namespace libcxx
{
void *memchr(const void *str, int c, size_t n);
void *memrchr(const void *str, int c, size_t n);
int memcmp(const void *str1, const void *str2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *str, int c, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strchr(const char *str, int c);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strdup(const char *str);
size_t strlen(const char *str);
char *strdup(const char *str);
char *strrchr(const char *str, int c);
char *strstr(const char *haystack, const char *needle);
char *strtok_r(char *str, const char *delim, char **lasts);
} // namespace libcxx
