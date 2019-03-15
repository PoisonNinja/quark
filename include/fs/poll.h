#pragma once

#include <lib/functional.h>
#include <lib/vector.h>

namespace scheduler
{
class wait_queue;
}

namespace filesystem
{
using nfds_t = unsigned int;

struct pollfd {
    int fd;
    short events;
    short revents;
};

#define POLLERR (1 << 0)
#define POLLHUP (1 << 1)
#define POLLNVAL (1 << 2)

#define POLLIN (1 << 3)
#define POLLRDNORM (1 << 4)
#define POLLRDBAND (1 << 5)
#define POLLPRI (1 << 6)
#define POLLOUT (1 << 7)
#define POLLWRNORM (1 << 8)
#define POLLWRBAND (1 << 9)

#define POLL__ONLY_REVENTS (POLLERR | POLLHUP | POLLNVAL)
class poll_table;

using poll_register_func_t = libcxx::function<void(scheduler::wait_queue&), 64>;

class poll_table
{
private:
    struct poll_table_elem {
        bool registered;
        struct pollfd* fd;
        scheduler::wait_queue* queue;
    };

public:
    poll_table(struct pollfd* fds, size_t size);
    ~poll_table();

    // TODO: Use timespec
    int poll(time_t timeout);

private:
    void bind(size_t offset, scheduler::wait_queue& queue);

    libcxx::vector<poll_table_elem> targets;
    struct pollfd* user_fds;
    size_t user_fds_size;
};
}; // namespace filesystem