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

#define POLLIN 1    /* Set if data to read. */
#define POLLPRI 2   /* Set if urgent data to read. */
#define POLLOUT 4   /* Set if writing data wouldn't block. */
#define POLLERR 8   /* An error occured. */
#define POLLHUP 16  /* Shutdown or close happened. */
#define POLLNVAL 32 /* Invalid file descriptor. */

#define NPOLLFILE 64 /* Number of canonical fd's in one call to poll(). */

/* The following values are defined by XPG4. */
#define POLLRDNORM POLLIN
#define POLLRDBAND POLLPRI
#define POLLWRNORM POLLOUT
#define POLLWRBAND POLLOUT

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