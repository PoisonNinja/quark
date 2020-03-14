#include <lib/queue.h>
#include <proc/sched.h>
#include <proc/work.h>
#include <proc/wq.h>

namespace scheduler
{
namespace work
{
namespace
{
struct work {
    work_handler_t handler;
    libcxx::node<work> node;
};

libcxx::queue<work, &work::node> work_queue;
scheduler::wait_queue worker_queue;

void do_work(void*)
{
    if (work_queue.empty()) {
        worker_queue.wait(wait_interruptible);
    } else {
        struct work& w = work_queue.front();
        work_queue.pop();
        w.handler();
        delete (&w);
    }
}
} // namespace

void schedule(work_handler_t handler)
{
    struct work* w = new struct work;
    w->handler     = handler;
    work_queue.push(*w);
}

void init()
{
    create_kernel_thread(do_work, nullptr);
}
} // namespace work
} // namespace scheduler
