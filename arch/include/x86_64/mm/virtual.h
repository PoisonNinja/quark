#pragma once

#include <types.h>

namespace memory
{
namespace virt
{
constexpr size_t PAGE_SIZE = 4096;

// No need to pack since it's already aligned
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
    uint64_t address : 40;
    uint32_t available_2 : 11;
    uint32_t nx : 1;
};

// x86_64 = 512 pages
struct page_table {
    struct page pages[PAGE_SIZE / sizeof(struct page)];
};
} // namespace virt
} // namespace memory
