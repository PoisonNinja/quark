#include <errno.h>
#include <fs/poll.h>
#include <lib/string.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/wait.h>

namespace filesystem
{
poll_table::poll_table(struct pollfd* fds, size_t size)
    : user_fds(fds)
    , user_fds_size(size)
{
    for (size_t i = 0; i < user_fds_size; i++) {
        poll_table_elem elem;
        elem.registered = false;
        elem.fd         = new pollfd;
        elem.queue      = nullptr;
        libcxx::memcpy(elem.fd, fds + i, sizeof(struct pollfd));
        targets.push_back(elem);
    }
} // namespace filesystem

poll_table::~poll_table()
{
}

int poll_table::poll(time_t timeout)
{
    auto proc   = scheduler::get_current_process();
    int revents = 0;
    // TODO: Use timer
    /*
     * The way this works is pretty funky. Every time we run this the poll
     * function for the descriptor will be called. However, what happens
     * is slightly different.
     *
     * The first time we run through the poll table, the descriptor's poll
     * function will call poll_table::bind. Since this is the first run, the
     * bind function will register the wait queue and all that stuff. Then, the
     * descriptor poll function will return any events. If there are any events
     * ready, we are done and can return.
     *
     * Most likely however, it won't be. We'll run through the rest of the poll
     * table, registering the wait queues as necessary. Then, we'll go to sleep.
     * When something happens (e.g. data becomes available), the driver will
     * call the wakeup function of it's wait queue. We'll get kicked back
     * into the top of the infinite loop.
     *
     * Then, we'll run through the entire table again, calling the poll function
     * for each descriptor. The descriptor will call poll_table::bind, but this
     * time nothing will happen because we've already registered. Instead, once
     * the descriptor's poll function returns we'll get events, which we can
     * then process.
     */
    while (1) {
        bool found = false;
        for (size_t i = 0; i < user_fds_size; i++) {
            auto desc = proc->fds.get(user_fds[i].fd);
            if (!desc) {
                // TODO: Correct errno?
                return -EINVAL;
            }
            poll_register_func_t callback = libcxx::bind(
                &poll_table::bind, this, i, libcxx::placeholders::_1);
            int res = desc->poll(callback);
            if (res) {
                // TODO: Respect pollfd->event
                found = true;
                // TODO: Sanitize this?
            }
            revents |= user_fds[i].revents = this->targets[i].fd->revents = res;
        }
        // No gotos :(
        if (found)
            break;
        scheduler::remove(scheduler::get_current_thread());
        scheduler::yield();
        // TODO: Check if a signal is the reason why we woke
    }
    // Now let's clean up :)
    for (auto& elem : targets) {
        // Remove us
        if (elem.queue) {
            elem.queue->remove();
        }
        delete elem.fd;
    }
    // TODO: Update the user fds
    return revents;
}

void poll_table::bind(size_t offset, scheduler::wait_queue& queue)
{
    if (!this->targets[offset].registered) {
        this->targets[offset].queue = &queue;
        queue.insert(scheduler::wait_interruptible);
        this->targets[offset].registered = true;
    }
}

}; // namespace filesystem