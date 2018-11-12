#pragma once

#include <lib/functional.h>
#include <lib/murmur.h>
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

class string;

template <>
struct hash<libcxx::string> {
    size_t operator()(const libcxx::string &s) const;
};

class string
{
public:
    string()
        : large_buffer(nullptr)
        , capacity(0)
        , buffer(nullptr)
        , size(0)
    {
    }
    string(const char *s)
    {
        this->cstr_to_int(s);
    }
    string(const string &s)
    {
        this->cstr_to_int(s.buffer);
    }

    string &operator=(const libcxx::string &other)
    {
        this->cstr_to_int(other.buffer);
        return *this;
    }

    bool operator==(const libcxx::string &other)
    {
        return (libcxx::strcmp(this->buffer, other.buffer) == 0);
    }
    bool operator!=(const libcxx::string &other)
    {
        return !(*this == other);
    }

    const char *c_str()
    {
        return this->buffer;
    }

private:
    ssize_t cstr_to_int(const char *s)
    {
        if (this->size > 15) {
            if (this->buffer) {
                delete[] this->buffer;
                this->capacity = 0;
            }
        }
        size_t s_size = libcxx::strlen(s);
        if (s_size <= 15) {
            libcxx::strncpy(this->small_buffer, s, 16);
            this->buffer = this->small_buffer;
        } else {
            this->buffer = new char[s_size + 1];
            libcxx::strcpy(this->buffer, s);
            this->capacity = s_size;
            this->buffer   = this->large_buffer;
        }
        return s_size;
    }
    union {
        struct {
            char *large_buffer;
            size_t capacity;
        };
        char small_buffer[16];
    };
    char *buffer;
    size_t size;

    friend size_t libcxx::hash<libcxx::string>::
    operator()(const libcxx::string &s) const;
};

inline size_t hash<libcxx::string>::operator()(const libcxx::string &s) const
{
    return Murmur::hash(s.buffer, s.size);
}
} // namespace libcxx
