#pragma once

#include <types.h>

namespace Memory
{
namespace Virtual
{
constexpr size_t PAGE_SIZE = 4096;

struct page {
    uint32_t present : 1;
    uint32_t writable : 1;
    uint32_t user : 1;
    uint32_t writethrough_cache : 1;
    uint32_t disable_cache : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t huge_page : 1;
    uint32_t global : 1;
    uint32_t cow : 1;
    uint32_t hardware : 1;
    uint32_t available_1 : 1;
    uint32_t address : 20;
} __attribute__((packed));

struct page_table {
    struct page pages[1024];
} __attribute__((packed));
}
}
