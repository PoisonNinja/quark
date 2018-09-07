#include <arch/mm/layout.h>
#include <arch/mm/mm.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>

namespace
{
static addr_t heap_end = HEAP_START;

void *map_heap(size_t size)
{
    size *= Memory::Virtual::PAGE_SIZE;
    Memory::Virtual::map_range(heap_end, size, PAGE_WRITABLE);
    heap_end += size;
    return reinterpret_cast<void *>(heap_end - size);
}

void free_heap(void *start, size_t size)
{
    size *= Memory::Virtual::PAGE_SIZE;
    for (size_t i = 0; i < size; i += Memory::Virtual::PAGE_SIZE) {
        Memory::Virtual::unmap_range(reinterpret_cast<addr_t>(start), size);
    }
}

int liballoc_lock(void)
{
    // spin_lock(heap_lock);
    return 0;
}

int liballoc_unlock(void)
{
    // spin_unlock(heap_lock);
    return 0;
}
} // namespace

#define VERSION "1.1"
#define ALIGNMENT                                                              \
    16ul // 4ul				///< This is the byte alignment that memory must
         // be
         // allocated on. IMPORTANT for GTK and other stuff.

#define ALIGN_TYPE char /// unsigned char[16] /// unsigned short
#define ALIGN_INFO                                                             \
    sizeof(ALIGN_TYPE) * 16 ///< Alignment information is stored right before
                            /// the pointer. This is the number of bytes of
/// information stored there.

/** This macro will conveniently align our pointer upwards */
#define ALIGN(ptr)                                                             \
    if (ALIGNMENT > 1) {                                                       \
        uintptr_t diff;                                                        \
        ptr  = (void *)((uintptr_t)ptr + ALIGN_INFO);                          \
        diff = (uintptr_t)ptr & (ALIGNMENT - 1);                               \
        if (diff != 0) {                                                       \
            diff = ALIGNMENT - diff;                                           \
            ptr  = (void *)((uintptr_t)ptr + diff);                            \
        }                                                                      \
        *((ALIGN_TYPE *)((uintptr_t)ptr - ALIGN_INFO)) = diff + ALIGN_INFO;    \
    }

#define UNALIGN(ptr)                                                           \
    if (ALIGNMENT > 1) {                                                       \
        uintptr_t diff = *((ALIGN_TYPE *)((uintptr_t)ptr - ALIGN_INFO));       \
        if (diff < (ALIGNMENT + ALIGN_INFO)) {                                 \
            ptr = (void *)((uintptr_t)ptr - diff);                             \
        }                                                                      \
    }

#define LIBALLOC_MAGIC 0xc001c0de
#define LIBALLOC_DEAD 0xdeaddead

/** A structure found at the top of all system allocated
 * memory blocks. It details the usage of the memory block.
 */
struct liballoc_major {
    struct liballoc_major *prev; ///< Linked list information.
    struct liballoc_major *next; ///< Linked list information.
    unsigned int pages;          ///< The number of pages in the block.
    unsigned int size;           ///< The number of pages in the block.
    unsigned int usage;          ///< The number of bytes used in the block.
    struct liballoc_minor
        *first; ///< A pointer to the first allocated memory in the block.
};

/** This is a structure found at the beginning of all
 * sections in a major block which were allocated by a
 * malloc, calloc, realloc call.
 */
struct liballoc_minor {
    struct liballoc_minor *prev; ///< Linked list information.
    struct liballoc_minor *next; ///< Linked list information.
    struct liballoc_major
        *block;         ///< The owning block. A pointer to the major structure.
    unsigned int magic; ///< A magic number to idenfity correctness.
    unsigned int size;  ///< The size of the memory allocated. Could be 1
                        ///< byte or more.
    unsigned int req_size; ///< The size of memory requested.
};

static struct liballoc_major *l_memRoot =
    nullptr; ///< The root memory block acquired from the system.
static struct liballoc_major *l_bestBet =
    nullptr; ///< The major with the most free memory.

static unsigned int l_pageSize =
    4096; ///< The size of an individual page. Set up in liballoc_init.
static unsigned int l_pageCount = 16; ///< The number of pages to request
                                      ///< per chunk. Set up in
                                      ///< liballoc_init.
static unsigned long long l_allocated =
    0;                                 ///< Running total of allocated memory.
static unsigned long long l_inuse = 0; ///< Running total of used memory.

static long long l_warningCount     = 0; ///< Number of warnings encountered
static long long l_errorCount       = 0; ///< Number of actual errors
static long long l_possibleOverruns = 0; ///< Number of possible overruns

// ***************************************************************

static struct liballoc_major *allocate_new_page(unsigned int size)
{
    unsigned int st;
    struct liballoc_major *maj;

    // This is how much space is required.
    st = size + sizeof(struct liballoc_major);
    st += sizeof(struct liballoc_minor);

    // Perfect amount of space?
    if ((st % l_pageSize) == 0)
        st = st / (l_pageSize);
    else
        st = st / (l_pageSize) + 1;
    // No, add the buffer.

    // Make sure it's >= the minimum size.
    if (st < l_pageCount)
        st = l_pageCount;

    maj = (struct liballoc_major *)map_heap(st);

    if (maj == nullptr) {
        l_warningCount += 1;
        return nullptr; // uh oh, we ran out of memory.
    }

    maj->prev  = nullptr;
    maj->next  = nullptr;
    maj->pages = st;
    maj->size  = st * l_pageSize;
    maj->usage = sizeof(struct liballoc_major);
    maj->first = nullptr;

    l_allocated += maj->size;

    return maj;
}

static void *__attribute__((malloc)) malloc(size_t req_size)
{
    int startedBet              = 0;
    unsigned long long bestSize = 0;
    void *p                     = nullptr;
    uintptr_t diff;
    struct liballoc_major *maj;
    struct liballoc_minor *min;
    struct liballoc_minor *new_min;
    unsigned long size = req_size;

    // For alignment, we adjust size so there's enough space to align.
    if (ALIGNMENT > 1) {
        size += ALIGNMENT + ALIGN_INFO;
    }
    // So, ideally, we really want an alignment of 0 or 1 in order
    // to save space.

    liballoc_lock();

    if (size == 0) {
        l_warningCount += 1;
        liballoc_unlock();
        return malloc(1);
    }

    if (l_memRoot == nullptr) {
        // This is the first time we are being used.
        l_memRoot = allocate_new_page(size);
        if (l_memRoot == nullptr) {
            liballoc_unlock();
            return nullptr;
        }
    }
    // Now we need to bounce through every major and find enough space....

    maj        = l_memRoot;
    startedBet = 0;

    // Start at the best bet....
    if (l_bestBet != nullptr) {
        bestSize = l_bestBet->size - l_bestBet->usage;

        if (bestSize > (size + sizeof(struct liballoc_minor))) {
            maj        = l_bestBet;
            startedBet = 1;
        }
    }

    while (maj != nullptr) {
        diff = maj->size - maj->usage;
        // free memory in the block

        if (bestSize < diff) {
            // Hmm.. this one has more memory then our bestBet. Remember!
            l_bestBet = maj;
            bestSize  = diff;
        }

        // CASE 1:  There is not enough space in this major block.
        if (diff < (size + sizeof(struct liballoc_minor))) {
            // Another major block next to this one?
            if (maj->next != nullptr) {
                maj = maj->next; // Hop to that one.
                continue;
            }

            if (startedBet == 1) // If we started at the best bet,
            {                    // let's start all over again.
                maj        = l_memRoot;
                startedBet = 0;
                continue;
            }

            // Create a new major block next to this one and...
            maj->next = allocate_new_page(size); // next one will be okay.
            if (maj->next == nullptr)
                break; // no more memory.
            maj->next->prev = maj;
            maj             = maj->next;

            // .. fall through to CASE 2 ..
        }

        // CASE 2: It's a brand new block.
        if (maj->first == nullptr) {
            maj->first =
                (struct liballoc_minor *)((uintptr_t)maj +
                                          sizeof(struct liballoc_major));

            maj->first->magic    = LIBALLOC_MAGIC;
            maj->first->prev     = nullptr;
            maj->first->next     = nullptr;
            maj->first->block    = maj;
            maj->first->size     = size;
            maj->first->req_size = req_size;
            maj->usage += size + sizeof(struct liballoc_minor);

            l_inuse += size;

            p = (void *)((uintptr_t)(maj->first) +
                         sizeof(struct liballoc_minor));

            ALIGN(p);

            liballoc_unlock(); // release the lock
            return p;
        }

        // CASE 3: Block in use and enough space at the start of the block.
        diff = (uintptr_t)(maj->first);
        diff -= (uintptr_t)maj;
        diff -= sizeof(struct liballoc_major);

        if (diff >= (size + sizeof(struct liballoc_minor))) {
            // Yes, space in front. Squeeze in.
            maj->first->prev =
                (struct liballoc_minor *)((uintptr_t)maj +
                                          sizeof(struct liballoc_major));
            maj->first->prev->next = maj->first;
            maj->first             = maj->first->prev;

            maj->first->magic    = LIBALLOC_MAGIC;
            maj->first->prev     = nullptr;
            maj->first->block    = maj;
            maj->first->size     = size;
            maj->first->req_size = req_size;
            maj->usage += size + sizeof(struct liballoc_minor);

            l_inuse += size;

            p = (void *)((uintptr_t)(maj->first) +
                         sizeof(struct liballoc_minor));
            ALIGN(p);

            liballoc_unlock(); // release the lock
            return p;
        }

        // CASE 4: There is enough space in this block. But is it
        // contiguous?
        min = maj->first;

        // Looping within the block now...
        while (min != nullptr) {
            // CASE 4.1: End of minors in a block. Space from last and end?
            if (min->next == nullptr) {
                // the rest of this block is free...  is it big enough?
                diff = (uintptr_t)(maj) + maj->size;
                diff -= (uintptr_t)min;
                diff -= sizeof(struct liballoc_minor);
                diff -= min->size;
                // minus already existing usage..

                if (diff >= (size + sizeof(struct liballoc_minor))) {
                    // yay....
                    min->next =
                        (struct liballoc_minor *)((uintptr_t)min +
                                                  sizeof(
                                                      struct liballoc_minor) +
                                                  min->size);
                    min->next->prev = min;
                    min             = min->next;
                    min->next       = nullptr;
                    min->magic      = LIBALLOC_MAGIC;
                    min->block      = maj;
                    min->size       = size;
                    min->req_size   = req_size;
                    maj->usage += size + sizeof(struct liballoc_minor);

                    l_inuse += size;

                    p = (void *)((uintptr_t)min +
                                 sizeof(struct liballoc_minor));
                    ALIGN(p);

                    liballoc_unlock(); // release the lock
                    return p;
                }
            }

            // CASE 4.2: Is there space between two minors?
            if (min->next != nullptr) {
                // is the difference between here and next big enough?
                diff = (uintptr_t)(min->next);
                diff -= (uintptr_t)min;
                diff -= sizeof(struct liballoc_minor);
                diff -= min->size;
                // minus our existing usage.

                if (diff >= (size + sizeof(struct liballoc_minor))) {
                    // yay......
                    new_min =
                        (struct liballoc_minor *)((uintptr_t)min +
                                                  sizeof(
                                                      struct liballoc_minor) +
                                                  min->size);

                    new_min->magic    = LIBALLOC_MAGIC;
                    new_min->next     = min->next;
                    new_min->prev     = min;
                    new_min->size     = size;
                    new_min->req_size = req_size;
                    new_min->block    = maj;
                    min->next->prev   = new_min;
                    min->next         = new_min;
                    maj->usage += size + sizeof(struct liballoc_minor);

                    l_inuse += size;

                    p = (void *)((uintptr_t)new_min +
                                 sizeof(struct liballoc_minor));
                    ALIGN(p);

                    liballoc_unlock(); // release the lock
                    return p;
                }
            } // min->next != nullptr

            min = min->next;
        } // while min != nullptr ...

        // CASE 5: Block full! Ensure next block and loop.
        if (maj->next == nullptr) {
            if (startedBet == 1) {
                maj        = l_memRoot;
                startedBet = 0;
                continue;
            }

            // we've run out. we need more...
            maj->next =
                allocate_new_page(size); // next one guaranteed to be okay
            if (maj->next == nullptr)
                break; //  uh oh,  no more memory.....
            maj->next->prev = maj;
        }

        maj = maj->next;
    } // while (maj != nullptr)

    liballoc_unlock(); // release the lock
    return nullptr;
}

static void free(void *ptr)
{
    struct liballoc_minor *min;
    struct liballoc_major *maj;

    if (ptr == nullptr) {
        l_warningCount += 1;
        return;
    }

    UNALIGN(ptr);

    liballoc_lock(); // lockit

    min = (struct liballoc_minor *)((uintptr_t)ptr -
                                    sizeof(struct liballoc_minor));

    if (min->magic != LIBALLOC_MAGIC) {
        l_errorCount += 1;

        // Check for overrun errors. For all bytes of LIBALLOC_MAGIC
        if (((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
            ((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF)) ||
            ((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF))) {
            l_possibleOverruns += 1;
        }

        // being lied to...
        liballoc_unlock(); // release the lock
        return;
    }

    maj = min->block;

    l_inuse -= min->size;

    maj->usage -= (min->size + sizeof(struct liballoc_minor));
    min->magic = LIBALLOC_DEAD; // No mojo.

    if (min->next != nullptr)
        min->next->prev = min->prev;
    if (min->prev != nullptr)
        min->prev->next = min->next;

    if (min->prev == nullptr)
        maj->first = min->next;
    // Might empty the block. This was the first
    // minor.

    // We need to clean up after the majors now....

    if (maj->first == nullptr) // Block completely unused.
    {
        if (l_memRoot == maj)
            l_memRoot = maj->next;
        if (l_bestBet == maj)
            l_bestBet = nullptr;
        if (maj->prev != nullptr)
            maj->prev->next = maj->next;
        if (maj->next != nullptr)
            maj->next->prev = maj->prev;
        l_allocated -= maj->size;

        free_heap(maj, maj->pages);
    } else {
        if (l_bestBet != nullptr) {
            int bestSize = l_bestBet->size - l_bestBet->usage;
            int majSize  = maj->size - maj->usage;

            if (majSize > bestSize)
                l_bestBet = maj;
        }
    }

    liballoc_unlock(); // release the lock
}

void *operator new(size_t size)
{
    return malloc(size);
}

void *operator new[](size_t size)
{
    return ::operator new(size);
}

void operator delete(void *p)
{
    free(p);
}

void operator delete(void *p, size_t)
{
    ::operator delete(p);
}

void operator delete[](void *p)
{
    ::operator delete(p);
}

void operator delete[](void *p, size_t)
{
    ::operator delete(p);
}
